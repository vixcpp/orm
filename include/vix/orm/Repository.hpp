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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace vix::orm
{
  /**
   * @brief Generic repository for ORM entities.
   *
   * BaseRepository<T> provides a small, explicit, and predictable CRUD API
   * for a single entity type mapped to a single database table.
   *
   * Assumptions:
   * - the table has a primary key column named "id"
   * - Mapper<T> is specialized by the user
   * - SQL stays explicit and deterministic
   *
   * Design goals:
   * - no hidden queries
   * - no reflection
   * - no implicit dirty tracking
   * - no runtime magic
   */
  template <class T>
  class BaseRepository
  {
    vix::db::ConnectionPool &pool_;
    std::string table_;

    static void ensureNotEmpty(const FieldValues &fields,
                               const char *context)
    {
      if (fields.empty())
      {
        throw std::runtime_error(std::string("BaseRepository: empty field list in ") + context);
      }
    }

    static std::string buildInsertColumns(const FieldValues &fields)
    {
      std::string cols;
      cols.reserve(64);

      for (std::size_t i = 0; i < fields.size(); ++i)
      {
        cols += fields[i].first;
        if (i + 1 < fields.size())
        {
          cols += ",";
        }
      }

      return cols;
    }

    static std::string buildInsertPlaceholders(std::size_t n)
    {
      std::string placeholders;
      placeholders.reserve(32);

      for (std::size_t i = 0; i < n; ++i)
      {
        placeholders += "?";
        if (i + 1 < n)
        {
          placeholders += ",";
        }
      }

      return placeholders;
    }

    static std::string buildUpdateSetClause(const FieldValues &fields)
    {
      std::string setClause;
      setClause.reserve(128);

      for (std::size_t i = 0; i < fields.size(); ++i)
      {
        setClause += fields[i].first;
        setClause += "=?";
        if (i + 1 < fields.size())
        {
          setClause += ",";
        }
      }

      return setClause;
    }

    static void bindFields(vix::db::Statement &st,
                           const FieldValues &fields,
                           std::size_t startIndex = 1)
    {
      std::size_t index = startIndex;
      for (const auto &field : fields)
      {
        st.bind(index++, any_to_dbvalue_or_throw(field.second));
      }
    }

  public:
    /**
     * @brief Construct a repository bound to a table.
     *
     * @param pool  Connection pool used for queries.
     * @param table Database table name.
     */
    BaseRepository(vix::db::ConnectionPool &pool, std::string table)
        : pool_(pool), table_(std::move(table))
    {
      if (table_.empty())
      {
        throw std::runtime_error("BaseRepository: table name cannot be empty");
      }
    }

    /**
     * @brief Return the database table name used by this repository.
     *
     * @return Table name.
     */
    const std::string &table() const noexcept
    {
      return table_;
    }

    /**
     * @brief Access the underlying connection pool.
     *
     * @return Connection pool reference.
     */
    vix::db::ConnectionPool &pool() noexcept
    {
      return pool_;
    }

    /**
     * @brief Access the underlying connection pool (const).
     *
     * @return Connection pool reference.
     */
    const vix::db::ConnectionPool &pool() const noexcept
    {
      return pool_;
    }

    /**
     * @brief Insert a new entity.
     *
     * Uses Mapper<T>::toInsertFields to generate the INSERT statement.
     *
     * @param value Entity instance.
     * @return Generated primary key.
     */
    std::uint64_t create(const T &value)
    {
      const auto fields = Mapper<T>::toInsertFields(value);
      ensureNotEmpty(fields, "create");

      const std::string columns = buildInsertColumns(fields);
      const std::string placeholders = buildInsertPlaceholders(fields.size());

      const std::string sql =
          "INSERT INTO " + table_ + " (" + columns + ") VALUES (" + placeholders + ")";

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);

      bindFields(*st, fields);
      st->exec();

      return conn.get().lastInsertId();
    }

    /**
     * @brief Find an entity by primary key.
     *
     * @param id Primary key value.
     * @return Entity if found, otherwise std::nullopt.
     */
    std::optional<T> findById(std::int64_t id)
    {
      const std::string sql =
          "SELECT * FROM " + table_ + " WHERE id = ? LIMIT 1";

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);
      st->bind(1, id);

      auto rs = st->query();
      if (!rs || !rs->next())
      {
        return std::nullopt;
      }

      return Mapper<T>::fromRow(rs->row());
    }

    /**
     * @brief Return all rows from the table.
     *
     * @return Vector of materialized entities.
     */
    std::vector<T> findAll()
    {
      const std::string sql = "SELECT * FROM " + table_;

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);
      auto rs = st->query();

      std::vector<T> out;
      while (rs && rs->next())
      {
        out.push_back(Mapper<T>::fromRow(rs->row()));
      }

      return out;
    }

    /**
     * @brief Check whether an entity exists for a given primary key.
     *
     * @param id Primary key value.
     * @return true if a row exists, false otherwise.
     */
    bool existsById(std::int64_t id)
    {
      const std::string sql =
          "SELECT id FROM " + table_ + " WHERE id = ? LIMIT 1";

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);
      st->bind(1, id);

      auto rs = st->query();
      return rs && rs->next();
    }

    /**
     * @brief Count all rows in the table.
     *
     * @return Total number of rows.
     */
    std::uint64_t count()
    {
      const std::string sql = "SELECT COUNT(*) FROM " + table_;

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);
      auto rs = st->query();

      if (!rs || !rs->next())
      {
        return 0;
      }

      return static_cast<std::uint64_t>(rs->row().getInt64(0));
    }

    /**
     * @brief Update an entity by primary key.
     *
     * Uses Mapper<T>::toUpdateFields to generate the SET clause.
     *
     * @param id    Primary key value.
     * @param value Entity instance.
     * @return Number of affected rows.
     */
    std::uint64_t updateById(std::int64_t id, const T &value)
    {
      const auto fields = Mapper<T>::toUpdateFields(value);
      ensureNotEmpty(fields, "updateById");

      const std::string setClause = buildUpdateSetClause(fields);
      const std::string sql =
          "UPDATE " + table_ + " SET " + setClause + " WHERE id=?";

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);

      bindFields(*st, fields);
      st->bind(fields.size() + 1, id);

      return st->exec();
    }

    /**
     * @brief Delete an entity by primary key.
     *
     * @param id Primary key value.
     * @return Number of affected rows.
     */
    std::uint64_t removeById(std::int64_t id)
    {
      const std::string sql =
          "DELETE FROM " + table_ + " WHERE id = ?";

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);
      st->bind(1, id);

      return st->exec();
    }

    /**
     * @brief Delete all rows from the table.
     *
     * @return Number of affected rows.
     */
    std::uint64_t removeAll()
    {
      const std::string sql = "DELETE FROM " + table_;

      vix::db::PooledConn conn(pool_);
      auto st = conn.get().prepare(sql);

      return st->exec();
    }
  };

} // namespace vix::orm

#endif // VIX_REPOSITORY_HPP
