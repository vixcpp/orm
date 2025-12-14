
#include <vix/orm/Errors.hpp>
#if VIX_ORM_HAS_MYSQL
#include <vix/orm/MySQLDriver.hpp>

#include <cppconn/statement.h>
#include <mysql_driver.h>
#include <memory>
#include <typeinfo>

namespace vix::orm
{
    class MySQLResultRow final : public ResultRow
    {
        std::shared_ptr<sql::ResultSet> rs_;
        [[maybe_unused]] std::size_t idx_;

    public:
        MySQLResultRow(std::shared_ptr<sql::ResultSet> rs, std::size_t idx)
            : rs_(std::move(rs)), idx_(idx) {}

        bool isNull(std::size_t i) const override
        {
            return rs_->isNull(static_cast<unsigned int>(i + 1));
        }

        std::string getString(std::size_t i) const override
        {
            return rs_->getString(static_cast<unsigned int>(i + 1));
        }

        std::int64_t getInt64(std::size_t i) const override
        {
            return static_cast<std::int64_t>(
                rs_->getInt64(static_cast<unsigned int>(i + 1)));
        }

        double getDouble(std::size_t i) const override
        {
            auto v = rs_->getDouble(static_cast<unsigned int>(i + 1));
            return static_cast<double>(v);
        }
    };

    class MySQLResultSet final : public ResultSet
    {
        std::shared_ptr<sql::ResultSet> rs_;

    public:
        explicit MySQLResultSet(std::shared_ptr<sql::ResultSet> rs)
            : rs_(std::move(rs)) {}

        bool next() override
        {
            return rs_->next();
        }

        std::size_t cols() const override
        {
            return static_cast<std::size_t>(rs_->getMetaData()->getColumnCount());
        }

        std::unique_ptr<ResultRow> row() const override
        {
            return std::make_unique<MySQLResultRow>(rs_, 0);
        }
    };

    class MySQLStatement final : public Statement
    {
        std::unique_ptr<sql::PreparedStatement> ps_;

        static unsigned int ui(std::size_t i)
        {
            // SQL params are 1-based: 1,2,3...
            return static_cast<unsigned int>(i);
        }

    public:
        explicit MySQLStatement(std::unique_ptr<sql::PreparedStatement> ps)
            : ps_(std::move(ps)) {}

        void bind(std::size_t idx, const std::any &v) override
        {
            const auto i = ui(idx);
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

        std::unique_ptr<ResultSet> query() override
        {
            try
            {
                auto raw = std::shared_ptr<sql::ResultSet>(ps_->executeQuery());
                return std::make_unique<MySQLResultSet>(std::move(raw));
            }
            catch (const sql::SQLException &e)
            {
                throw DBError(std::string{"MySQL query failed: "} + e.what());
            }
        }

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

    std::unique_ptr<Statement> MySQLConnection::prepare(std::string_view sql)
    {
        try
        {
            auto ps = std::unique_ptr<sql::PreparedStatement>(
                conn_->prepareStatement(std::string(sql)));
            return std::make_unique<MySQLStatement>(std::move(ps));
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL prepare failed: "} + e.what());
        }
    }

    std::uint64_t MySQLConnection::lastInsertId()
    {
        try
        {
            auto st = std::unique_ptr<sql::Statement>(conn_->createStatement());
            auto rs = std::unique_ptr<sql::ResultSet>(
                st->executeQuery("SELECT LAST_INSERT_ID() AS id"));
            if (rs->next())
                return rs->getUInt64("id");
            throw DBError("No LAST_INSERT_ID()");
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL lastInsertId failed: "} + e.what());
        }
    }

    std::shared_ptr<sql::Connection> make_mysql_conn(const std::string &host,
                                                     const std::string &user,
                                                     const std::string &pass,
                                                     const std::string &db)
    {
        try
        {
            auto *driver = sql::mysql::get_mysql_driver_instance();
            auto c = std::shared_ptr<sql::Connection>(driver->connect(host, user, pass));
            if (!db.empty())
                c->setSchema(db);
            return c;
        }
        catch (const sql::SQLException &e)
        {
            throw DBError(std::string{"MySQL connect failed: "} + e.what());
        }
    }

    std::function<std::shared_ptr<Connection>()>
    make_mysql_factory(std::string host,
                       std::string user,
                       std::string pass,
                       std::string db)
    {
        return [host = std::move(host),
                user = std::move(user),
                pass = std::move(pass),
                db = std::move(db)]() -> std::shared_ptr<Connection>
        {
            auto raw = make_mysql_conn(host, user, pass, db);
            auto mysql_conn = std::make_shared<MySQLConnection>(std::move(raw));
            return std::static_pointer_cast<Connection>(mysql_conn);
        };
    }

} // namespace vix::orm

#endif