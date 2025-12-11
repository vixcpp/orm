#include <vix/orm/Database.hpp>
#include <vix/orm/MySQLDriver.hpp>
#include <vix/config/Config.hpp>

#include <stdexcept>
#include <utility>

namespace vix::orm
{
    ConnectionFactory make_mysql_factory(const MySQLConfig &cfg)
    {
        return [cfg]() -> ConnectionPtr
        {
            auto raw = make_mysql_conn(cfg.host, cfg.user, cfg.password, cfg.database);
            auto mysql_conn = std::make_shared<MySQLConnection>(std::move(raw));
            return std::static_pointer_cast<Connection>(mysql_conn);
        };
    }

    DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg)
    {
        DbConfig out;

        auto engine_str = cfg.getString("db.engine", "mysql");
        if (engine_str == "mysql")
        {
            out.engine = Engine::MySQL;
        }
        else if (engine_str == "sqlite")
        {
            out.engine = Engine::SQLite;
        }
        else
        {
            out.engine = Engine::MySQL;
        }

        out.mysql.host = cfg.getString("db.host", "tcp://127.0.0.1:3306");
        out.mysql.user = cfg.getString("db.user", "root");
        out.mysql.password = cfg.getString("db.password", "");
        out.mysql.database = cfg.getString("db.database", "vixdb");

        out.mysql.pool.min = static_cast<std::size_t>(cfg.getInt("db.pool.min", 1));
        out.mysql.pool.max = static_cast<std::size_t>(cfg.getInt("db.pool.max", 8));

        out.sqlite.path = cfg.getString("db.sqlite", "vix_orm.db");
        out.sqlite.pool = out.mysql.pool;

        return out;
    }

    namespace
    {
        ConnectionFactory make_factory_for(const DbConfig &cfg)
        {
            switch (cfg.engine)
            {
            case Engine::MySQL:
                return make_mysql_factory(cfg.mysql);

            case Engine::SQLite:
                throw std::runtime_error("SQLite engine not implemented yet in Vix ORM");

            default:
                throw std::runtime_error("Unsupported database engine in DbConfig");
            }
        }

        PoolConfig pool_for(const DbConfig &cfg)
        {
            switch (cfg.engine)
            {
            case Engine::MySQL:
                return cfg.mysql.pool;
            case Engine::SQLite:
                return cfg.sqlite.pool;
            default:
                return cfg.mysql.pool;
            }
        }
    } // namespace

    Database::Database(const DbConfig &cfg)
        : cfg_(cfg),
          pool_(make_factory_for(cfg), pool_for(cfg))
    {
        pool_.warmup();
    }

} // namespace vix::orm
