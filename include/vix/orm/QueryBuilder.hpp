/**
 *
 *  @file QueryBuilder.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */

#ifndef VIX_QUERY_BUILDER
#define VIX_QUERY_BUILDER

#include <string>
#include <string_view>
#include <vector>

#include <vix/db/core/Value.hpp>

namespace vix::orm
{
  class QueryBuilder
  {
    std::string sql_;
    std::vector<vix::db::DbValue> params_;

  public:
    QueryBuilder &raw(std::string_view s)
    {
      sql_.append(s);
      return *this;
    }

    QueryBuilder &space()
    {
      sql_.push_back(' ');
      return *this;
    }

    QueryBuilder &param(const vix::db::DbValue &v)
    {
      params_.push_back(v);
      return *this;
    }

    QueryBuilder &param(vix::db::DbValue &&v)
    {
      params_.push_back(std::move(v));
      return *this;
    }

    QueryBuilder &param(std::int64_t v) { return param(vix::db::i64(v)); }
    QueryBuilder &param(int v) { return param(vix::db::i64(static_cast<std::int64_t>(v))); }
    QueryBuilder &param(std::uint64_t v) { return param(vix::db::i64(static_cast<std::int64_t>(v))); }
    QueryBuilder &param(double v) { return param(vix::db::f64(v)); }
    QueryBuilder &param(bool v) { return param(vix::db::b(v)); }

    QueryBuilder &param(std::string v) { return param(vix::db::str(std::move(v))); }
    QueryBuilder &param(const char *v) { return param(vix::db::str(std::string(v ? v : ""))); }

    QueryBuilder &paramNull() { return param(vix::db::null()); }

    const std::string &sql() const { return sql_; }
    const std::vector<vix::db::DbValue> &params() const { return params_; }
  };
} // namespace vix::orm

#endif // VIX_QUERY_BUILDER
