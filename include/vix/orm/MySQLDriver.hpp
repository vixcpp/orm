#pragma once

#if VIX_ORM_HAS_MYSQL

#include <vix/orm/Drivers.hpp>

#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>

#include <memory>
#include <string>

namespace vix::orm
{
    class MySQLConnection final : public Connection
    {
        std::shared_ptr<sql::Connection> conn_;

    public:
        explicit MySQLConnection(std::shared_ptr<sql::Connection> c)
            : conn_(std::move(c)) {}

        std::unique_ptr<Statement> prepare(std::string_view sql) override;

        void begin() override
        {
            conn_->setAutoCommit(false);
        }

        void commit() override
        {
            conn_->commit();
            conn_->setAutoCommit(true);
        }

        void rollback() override
        {
            conn_->rollback();
            conn_->setAutoCommit(true);
        }

        std::uint64_t lastInsertId() override;

        const std::shared_ptr<sql::Connection> &raw() const { return conn_; }
    };

    std::shared_ptr<sql::Connection>
    make_mysql_conn(const std::string &host,
                    const std::string &user,
                    const std::string &pass,
                    const std::string &db);

    std::function<std::shared_ptr<Connection>()>
    make_mysql_factory(std::string host,
                       std::string user,
                       std::string pass,
                       std::string db);
}

#endif // VIX_ORM_HAS_MYSQL
