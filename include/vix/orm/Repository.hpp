/**
 *
 *  @file Repository.hpp
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
#ifndef VIX_REPOSITORY_HPP
#define VIX_REPOSITORY_HPP

#include <vix/orm/db_compat.hpp>
#include <vix/orm/Mapper.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vix::orm
{
  /**
   * @brief Generic repository for ORM entities.
   *
   * BaseRepository<T> provides a minimal CRUD interface for an
   * ORM entity mapped to a single database table.
   *
   * Assumptions:
   * - The underlying table uses a primary key column named "id"
   * - The Mapper<T> specialization defines how the entity is
   *   serialized/deserialized
   *
   * This class intentionally avoids complex query abstraction
   * and focuses on predictable, explicit SQL.
   */
  template <class T>
  class BaseRepository
  {
    vix::db::ConnectionPool &pool_;
    std::string table_;

    static std::string
    build_insert_cols(const std::vector<std::pair<std::string, std::any>> &params)
    {
      std::string cols;
      cols.reserve(64);

      for (std::size_t i = 0; i < params.size(); ++i)
      {
        cols += params[i].first;
        if (i + 1 < params.size())
          cols += ",";
      }
      return cols;
    }

    static std::string build_insert_qs(std::size_t n)
    {
      std::string qs;
      qs.reserve(32);

      for (std::size_t i = 0; i < n; ++i)
      {
        qs += "?";
        if (i + 1 < n)
          qs += ",";
      }
      return qs;
    }

    static std::string
    build_update_set(const std::vector<std::pair<std::string, std::any>> &params)
    {
      std::string set;
      set.reserve(128);

      for (std::size_t i = 0; i < params.size(); ++i)
      {
        set += params[i].first;
        set += "=?";
        if (i + 1 < params.size())
          set += ",";
      }
      return set;
    }

  public:
    /**
     * @brief Construct a repository bound to a table.
     *
     * @param pool  Connection pool used for queries.
     * @param table Database table name.
     */
    BaseRepository(vix::db::ConnectionPool &pool, std::string table)
        : pool_(pool), table_(std::move(table)) {}

    /**
     * @brief Insert a new entity.
     *
     * Uses Mapper<T>::toInsertParams to build the INSERT statement.
     *
     * @param v Entity instance.
     * @return Generated primary key (last insert id).
     */
    std::uint64_t create(const T &v)
    {
      const auto params = Mapper<T>::toInsertParams(v);

      const std::string cols = build_insert_cols(params);
      const std::string qs = build_insert_qs(params.size());

      const std::string sql =
          "INSERT INTO " + table_ + " (" + cols + ") VALUES (" + qs + ")";

      vix::db::PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);

      for (std::size_t i = 0; i < params.size(); ++i)
        st->bind(i + 1, any_to_dbvalue_or_throw(params[i].second));

      st->exec();
      return pc.get().lastInsertId();
    }

    /**
     * @brief Find an entity by its primary key.
     *
     * @param id Primary key value.
     * @return Entity instance if found, otherwise std::nullopt.
     */
    std::optional<T> findById(std::int64_t id)
    {
      const std::string sql =
          "SELECT * FROM " + table_ + " WHERE id = ? LIMIT 1";

      vix::db::PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);
      st->bind(1, id);

      auto rs = st->query();
      if (!rs || !rs->next())
        return std::nullopt;

      return Mapper<T>::fromRow(rs->row());
    }

    /**
     * @brief Update an entity by its primary key.
     *
     * Uses Mapper<T>::toUpdateParams to generate column assignments.
     *
     * @param id Primary key value.
     * @param v  Entity instance.
     * @return Number of affected rows.
     */
    std::uint64_t updateById(std::int64_t id, const T &v)
    {
      const auto params = Mapper<T>::toUpdateParams(v);

      const std::string set = build_update_set(params);
      const std::string sql =
          "UPDATE " + table_ + " SET " + set + " WHERE id=?";

      vix::db::PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);

      std::size_t idx = 1;
      for (const auto &kv : params)
        st->bind(idx++, any_to_dbvalue_or_throw(kv.second));

      st->bind(idx, id);
      return st->exec();
    }

    /**
     * @brief Delete an entity by its primary key.
     *
     * @param id Primary key value.
     * @return Number of affected rows.
     */
    std::uint64_t removeById(std::int64_t id)
    {
      vix::db::PooledConn pc(pool_);
      auto st = pc.get().prepare("DELETE FROM " + table_ + " WHERE id = ?");
      st->bind(1, id);
      return st->exec();
    }
  };

} // namespace vix::orm

#endif // VIX_REPOSITORY_HPP
