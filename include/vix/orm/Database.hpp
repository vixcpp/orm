/**
 *
 *  @file Database.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_ORM_DATABASE_HPP
#define VIX_ORM_DATABASE_HPP

#include <string>
#include <vix/orm/ConnectionPool.hpp>

namespace vix::config
{
  class Config;
}

namespace vix::orm
{
  enum class Engine
  {
    MySQL,
    SQLite
  };

  struct MySQLConfig
  {
    std::string host;     // ex: "tcp://127.0.0.1:3306"
    std::string user;     // ex: "root"
    std::string password; // ex: ""
    std::string database; // ex: "vixdb"
    PoolConfig pool{};    // min/max connections
  };

  struct SQLiteConfig
  {
    std::string path;  // ex: "vix_orm.db"
    PoolConfig pool{}; // reuse PoolConfig
  };

  struct DbConfig
  {
    Engine engine{Engine::MySQL};
    MySQLConfig mysql{};
    SQLiteConfig sqlite{};
  };

  ConnectionFactory make_mysql_factory(const MySQLConfig &cfg);

  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg);

  class Database
  {
  public:
    explicit Database(const DbConfig &cfg);
    Engine engine() const noexcept { return cfg_.engine; }
    const DbConfig &config() const noexcept { return cfg_; }
    ConnectionPool &pool() noexcept { return pool_; }
    const ConnectionPool &pool() const noexcept { return pool_; }

  private:
    DbConfig cfg_;
    ConnectionPool pool_;
  };

} // namespace vix::orm

#endif // VIX_ORM_DATABASE_HPP
