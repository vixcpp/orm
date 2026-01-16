/**
 *
 *  @file Repository.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_REPOSITORY_HPP
#define VIX_REPOSITORY_HPP

#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/Mapper.hpp>
#include <optional>
#include <vector>
#include <string>

namespace vix::orm
{
  template <class T>
  class BaseRepository
  {
    ConnectionPool &pool_;
    std::string table_;

  public:
    BaseRepository(ConnectionPool &pool, std::string table)
        : pool_(pool), table_(std::move(table)) {}

    std::uint64_t create(const T &v)
    {
      auto params = Mapper<T>::toInsertParams(v);

      std::string cols, qs;
      cols.reserve(64);
      qs.reserve(32);

      for (std::size_t i = 0; i < params.size(); ++i)
      {
        cols += params[i].first;
        qs += "?";
        if (i + 1 < params.size())
        {
          cols += ",";
          qs += ",";
        }
      }

      auto sql = "INSERT INTO " + table_ + " (" + cols + ") VALUES (" + qs + ")";
      PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);

      for (std::size_t i = 0; i < params.size(); ++i)
        st->bind(i + 1, params[i].second);

      st->exec();
      return pc.get().lastInsertId();
    }

    std::optional<T> findById(std::int64_t id)
    {
      auto sql = "SELECT * FROM " + table_ + " WHERE id = ? LIMIT 1";
      PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);
      st->bind(1, id);
      auto rs = st->query();

      return std::nullopt;
    }

    std::uint64_t updateById(std::int64_t id, const T &v)
    {
      auto params = Mapper<T>::toUpdateParams(v);

      std::string set;
      set.reserve(128);
      for (std::size_t i = 0; i < params.size(); ++i)
      {
        set += params[i].first + "=?";
        if (i + 1 < params.size())
          set += ",";
      }

      auto sql = "UPDATE " + table_ + " SET " + set + " WHERE id=?";
      PooledConn pc(pool_);
      auto st = pc.get().prepare(sql);

      std::size_t idx = 1;
      for (auto &[k, vv] : params)
        st->bind(idx++, vv);
      st->bind(idx, id);

      return st->exec();
    }

    std::uint64_t removeById(std::int64_t id)
    {
      PooledConn pc(pool_);
      auto st = pc.get().prepare("DELETE FROM " + table_ + " WHERE id = ?");
      st->bind(1, id);
      return st->exec();
    }
  };

} // namespace Vix::orm

#endif // VIX_REPOSITORY_HPP
