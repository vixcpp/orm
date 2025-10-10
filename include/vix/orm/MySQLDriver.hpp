#ifndef VIX_MYSQL_DRIVER_HPP
#define VIX_MYSQL_DRIVER_HPP

#include <vix/orm/Drivers.hpp>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <memory>
#include <string>

/**
 * @file MySQLDriver.hpp
 * @brief MySQL backend implementation for Vix ORM.
 *
 * This header defines the MySQL-specific implementation of the abstract `Connection`
 * interface from `Drivers.hpp`. It wraps the official **MySQL Connector/C++**
 * API to provide a lightweight, consistent driver layer.
 *
 * ---
 * ## Overview
 * `MySQLConnection` is a thin RAII wrapper around `sql::Connection` that:
 * - Implements `Connection` (for use with Vix ORM).
 * - Provides `prepare()` to create parameterized SQL statements.
 * - Manages transactions (`begin`, `commit`, `rollback`).
 * - Exposes `lastInsertId()` for auto-increment primary keys.
 *
 * ---
 * ## Architecture
 * ```
 * +------------------+       +--------------------+
 * | Base interface   |       |  MySQL implementation  |
 * |------------------|       |--------------------|
 * | Connection       | <---> | MySQLConnection     |
 * | Statement        | <---> | MySQLStatement      |
 * | ResultSet/Row    | <---> | MySQLResultSet/Row  |
 * +------------------+       +--------------------+
 * ```
 * All higher-level ORM layers (`Repository`, `QueryBuilder`, etc.)
 * depend only on `Connection`, never on MySQL-specific types.
 *
 * ---
 * ## Typical usage
 * ```cpp
 * #include <vix/orm/MySQLDriver.hpp>
 *
 * using namespace Vix::orm;
 *
 * int main() {
 *     // Create a shared connection via factory
 *     auto raw = make_mysql_conn("tcp://127.0.0.1:3306", "root", "secret", "softadastra");
 *
 *     MySQLConnection conn{raw};
 *     conn.begin();
 *     try {
 *         auto st = conn.prepare("INSERT INTO users (name, email) VALUES (?, ?)");
 *         st->bind(1, std::string("Alice"));
 *         st->bind(2, std::string("alice@example.com"));
 *         st->exec();
 *         conn.commit();
 *     } catch (...) {
 *         conn.rollback();
 *     }
 * }
 * ```
 *
 * ---
 * ## Connection Management
 * - Use `make_mysql_conn()` to create a fully connected `std::shared_ptr<sql::Connection>`.
 * - The driver automatically sets the active schema (`USE <db>`).
 * - In production code, prefer `ConnectionPool` for multi-threaded access.
 *
 * ---
 * ## Transactions
 * - `begin()` disables auto-commit until a `commit()` or `rollback()`.
 * - Each `commit()` automatically re-enables auto-commit.
 * - Use the `Transaction` RAII helper for safer scoping.
 *
 * ---
 * ## Notes
 * - This driver uses the official `mysqlcppconn` library.
 * - It requires linking against `MySQLCppConn::MySQLCppConn`.
 * - Thread-safety is **not guaranteed** on shared connections. Use pooling.
 *
 * @see Connection
 * @see ConnectionPool
 * @see Transaction
 * @see make_mysql_conn
 */

namespace Vix::orm
{
    /**
     * @brief Concrete MySQL implementation of the generic `Connection` interface.
     *
     * Provides prepared statements, transaction control, and last-insert ID retrieval.
     */
    class MySQLConnection final : public Connection
    {
        std::shared_ptr<sql::Connection> conn_; ///< Wrapped MySQL C++ connection object.

    public:
        /**
         * @brief Construct a `MySQLConnection` from an existing shared connection.
         *
         * @param c Shared pointer to a valid `sql::Connection`.
         */
        explicit MySQLConnection(std::shared_ptr<sql::Connection> c)
            : conn_(std::move(c)) {}

        /**
         * @brief Prepare a SQL statement.
         *
         * @param sql SQL query string (with `?` placeholders for parameters).
         * @return A driver-specific `Statement` wrapper.
         *
         * @throws DBError if preparation fails.
         */
        std::unique_ptr<Statement> prepare(std::string_view sql) override;

        /**
         * @brief Begin a transaction (disable auto-commit).
         */
        void begin() override { conn_->setAutoCommit(false); }

        /**
         * @brief Commit the current transaction and re-enable auto-commit.
         */
        void commit() override
        {
            conn_->commit();
            conn_->setAutoCommit(true);
        }

        /**
         * @brief Roll back the current transaction and re-enable auto-commit.
         */
        void rollback() override
        {
            conn_->rollback();
            conn_->setAutoCommit(true);
        }

        /**
         * @brief Return the ID of the last inserted row (auto-increment primary key).
         *
         * @return Last inserted ID as an unsigned 64-bit integer.
         */
        std::uint64_t lastInsertId() override;

        /**
         * @brief Access the raw MySQL connection (advanced users only).
         *
         * @return Reference to the underlying `std::shared_ptr<sql::Connection>`.
         */
        const std::shared_ptr<sql::Connection> &raw() const { return conn_; }
    };

    /**
     * @brief Utility function to create a connected MySQL handle.
     *
     * @param host MySQL host string (e.g. `"tcp://127.0.0.1:3306"`).
     * @param user Username for authentication.
     * @param pass Password for authentication.
     * @param db   Database schema name.
     *
     * @return Shared pointer to a connected `sql::Connection`.
     *
     * @throws DBError if the connection cannot be established.
     *
     * Example:
     * ```cpp
     * auto conn = make_mysql_conn("tcp://localhost:3306", "root", "pass", "softadastra");
     * ```
     */
    std::shared_ptr<sql::Connection> make_mysql_conn(const std::string &host,
                                                     const std::string &user,
                                                     const std::string &pass,
                                                     const std::string &db);

} // namespace Vix::orm

#endif // VIX_MYSQL_DRIVER_HPP
