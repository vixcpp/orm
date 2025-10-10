/**
 * @file MySQLDriver.cpp
 * @brief Contributor documentation — MySQL backend internals for Vix ORM.
 *
 * This file implements the MySQL-specific backend that satisfies the
 * abstract interfaces from Drivers.hpp:
 *   - Connection  → MySQLConnection
 *   - Statement   → MySQLStatement
 *   - ResultSet   → (planned) MySQLResultSet
 *   - ResultRow   → (planned) MySQLResultRow
 *
 * ───────────────────────────────────────────────────────────────────────────
 *  Design notes
 * ───────────────────────────────────────────────────────────────────────────
 * - Parameter indices are **1-based** (matches mysqlcppconn API).
 * - Column indices are **0-based** in ResultRow (consistent with Drivers.hpp).
 * - `bind()` inspects `std::any` and calls the proper mysqlcppconn setter.
 * - `query()` is not implemented yet → throws `DBError` as a clear signal.
 * - All driver calls that may throw `sql::SQLException` are wrapped and
 *   rethrown as `Vix::orm::DBError` to keep the exception hierarchy unified.
 *
 * TODOs / Roadmap:
 * - Implement MySQLResultSet / MySQLResultRow with column-count and getters.
 * - Add by-name accessors (requires column label → index map at prepare-time).
 * - Consider statement caching keyed by SQL for hot paths.
 * - Add connection/statement ping & health checks on reuse in ConnectionPool.
 */

#include <vix/orm/MySQLDriver.hpp>
#include <vix/orm/Errors.hpp>

#include <cppconn/statement.h>
#include <mysql_driver.h>
#include <memory>
#include <typeinfo>

namespace Vix::orm
{

    /**
     * @brief MySQL implementation of a prepared statement.
     *
     * Notes for contributors:
     * - Keep `bind()` fast; avoid allocations. We only inspect `std::any::type()`
     *   and call the corresponding setter.
     * - The helper `ui()` converts `std::size_t` to the unsigned int expected by
     *   mysqlcppconn parameter index APIs.
     * - `query()` is a placeholder until ResultSet/ResultRow adapters land.
     */
    class MySQLStatement final : public Statement
    {
        std::unique_ptr<sql::PreparedStatement> ps_;
        static unsigned int ui(std::size_t i) { return static_cast<unsigned int>(i); } // 1-based index → unsigned int

    public:
        explicit MySQLStatement(std::unique_ptr<sql::PreparedStatement> ps) : ps_(std::move(ps)) {}

        /**
         * @brief Bind a positional parameter (1-based).
         *
         * Supported `std::any` payload types and their mysqlcppconn mapping:
         * - `int`              → `setInt`
         * - `std::int64_t`     → `setInt64`
         * - `unsigned`         → `setUInt`
         * - `double`           → `setDouble`
         * - `float`            → `setDouble` (widen)
         * - `bool`             → `setBoolean`
         * - `const char*`      → `setString`
         * - `std::string`      → `setString`
         *
         * @throws DBError on unsupported types or driver failure.
         */
        void bind(std::size_t idx, const std::any &v) override
        {
            const auto i = ui(idx); // MySQL API uses unsigned int parameter index (1-based)
            try
            {
                if (v.type() == typeid(int))
                    ps_->setInt(i, std::any_cast<int>(v));
                else if (v.type() == typeid(std::int64_t))
                    ps_->setInt64(i, std::any_cast<std::int64_t>(v));
                else if (v.type() == typeid(unsigned))
                    ps_->setUInt(i, std::any_cast<unsigned>(v));
                else if (v.type() == typeid(double))
                    ps_->setDouble(i, std::any_cast<double>(v));
                else if (v.type() == typeid(float))
                    ps_->setDouble(i, static_cast<double>(std::any_cast<float>(v)));
                else if (v.type() == typeid(bool))
                    ps_->setBoolean(i, std::any_cast<bool>(v));
                else if (v.type() == typeid(const char *))
                    ps_->setString(i, std::any_cast<const char *>(v));
                else if (v.type() == typeid(std::string))
                    ps_->setString(i, std::any_cast<std::string>(v));
                else
                    throw DBError("Unsupported bind type in MySQLStatement::bind");
            }
            catch (const sql::SQLException &e)
            {
                throw DBError(std::string{"MySQL bind failed: "} + e.what());
            }
        }

        /**
         * @brief Execute a SELECT and return a forward-only result set.
         *
         * Not implemented yet: throws `DBError`.
         * When implemented, it should wrap `ps_->executeQuery()` and adapt the
         * resulting `sql::ResultSet` into a `Vix::orm::ResultSet`.
         */
        std::unique_ptr<ResultSet> query() override
        {
            // Placeholder until MySQLResultSet / MySQLResultRow are implemented.
            throw DBError("ResultSet adapter not implemented yet");
        }

        /**
         * @brief Execute a DML/DDL statement (INSERT/UPDATE/DELETE/DDL).
         * @return Affected row count (driver semantics).
         */
        std::uint64_t exec() override
        {
            try
            {
                return static_cast<std::uint64_t>(ps_->executeUpdate());
            }
            catch (const sql::SQLException &e)
            {
                throw DBError(std::string{"MySQL exec failed: "} + e.what());
            }
        }
    };

    /**
     * @brief Prepare a MySQL statement.
     *
     * Wraps `sql::Connection::prepareStatement` and returns a Vix `Statement`.
     * @throws DBError on `sql::SQLException`.
     */
    std::unique_ptr<Statement> MySQLConnection::prepare(std::string_view sql)
    {
        try
        {
            return std::make_unique<MySQLStatement>(
                std::unique_ptr<sql::PreparedStatement>(raw()->prepareStatement(std::string(sql))));
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL prepare failed: "} + e.what());
        }
    }

    /**
     * @brief Retrieve the last auto-increment ID for this session.
     * @throws DBError if the driver returns no row or on SQL error.
     */
    std::uint64_t MySQLConnection::lastInsertId()
    {
        try
        {
            auto st = raw()->createStatement();
            auto rs = std::unique_ptr<sql::ResultSet>(st->executeQuery("SELECT LAST_INSERT_ID() AS id"));
            if (rs->next())
                return rs->getUInt64("id");
            throw DBError("No LAST_INSERT_ID()");
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL lastInsertId failed: "} + e.what());
        }
    }

    /**
     * @brief Factory: connect to MySQL and select the target schema.
     *
     * @note Keep this minimal; connection pooling is handled elsewhere.
     * @throws DBError on connection failure.
     */
    std::shared_ptr<sql::Connection> make_mysql_conn(const std::string &host,
                                                     const std::string &user,
                                                     const std::string &pass,
                                                     const std::string &db)
    {
        try
        {
            auto *driver = sql::mysql::get_mysql_driver_instance();
            auto c = std::shared_ptr<sql::Connection>(driver->connect(host, user, pass));
            c->setSchema(db);
            return c;
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL connect failed: "} + e.what());
        }
    }

} // namespace Vix::orm
