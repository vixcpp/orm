/**
 *
 *  @file orm.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_ORM_HPP
#define VIX_ORM_HPP

#include <vix/orm/db_compat.hpp>
#include <vix/orm/Entity.hpp>
#include <vix/orm/Mapper.hpp>
#include <vix/orm/QueryBuilder.hpp>
#include <vix/orm/Repository.hpp>
#include <vix/orm/UnitOfWork.hpp>

#include <string>
#include <utility>

namespace vix::orm
{
  /**
   * @brief Convenience alias for the default repository type.
   *
   * This alias exists to make user code shorter and clearer when
   * working with standard ORM repositories.
   *
   * Example:
   * @code
   * vix::orm::Repository<User> repo(db.pool(), "users");
   * @endcode
   */
  template <class T>
  using Repository = BaseRepository<T>;

  /**
   * @brief Create a repository bound to a connection pool.
   *
   * This helper reduces boilerplate and keeps examples concise.
   *
   * @tparam T Entity type.
   * @param pool Connection pool.
   * @param table Database table name.
   * @return Repository instance.
   */
  template <class T>
  [[nodiscard]] inline BaseRepository<T>
  repository(vix::db::ConnectionPool &pool, std::string table)
  {
    return BaseRepository<T>(pool, std::move(table));
  }

  /**
   * @brief Create a repository bound to a database facade.
   *
   * This is the preferred high-level entry point when using the
   * vix::db::Database facade.
   *
   * @tparam T Entity type.
   * @param db Database facade.
   * @param table Database table name.
   * @return Repository instance.
   */
  template <class T>
  [[nodiscard]] inline BaseRepository<T>
  repository(vix::db::Database &db, std::string table)
  {
    return BaseRepository<T>(db.pool(), std::move(table));
  }

  /**
   * @brief Create a unit of work from a database facade.
   *
   * This helper keeps transaction setup concise and explicit.
   *
   * @param db Database facade.
   * @return UnitOfWork instance.
   */
  [[nodiscard]] inline UnitOfWork unit_of_work(vix::db::Database &db)
  {
    return UnitOfWork(db);
  }

  /**
   * @brief Create a unit of work from a connection pool.
   *
   * @param pool Connection pool.
   * @return UnitOfWork instance.
   */
  [[nodiscard]] inline UnitOfWork unit_of_work(vix::db::ConnectionPool &pool)
  {
    return UnitOfWork(pool);
  }

} // namespace vix::orm

#endif // VIX_ORM_HPP
