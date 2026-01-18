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
#include <any>

namespace vix::orm
{
  class QueryBuilder
  {
    std::string sql_;
    std::vector<std::any> params_;

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

    QueryBuilder &param(std::any v)
    {
      params_.push_back(std::move(v));
      return *this;
    }

    const std::string &sql() const { return sql_; }
    const std::vector<std::any> &params() const { return params_; }
  };
} // namespace Vix::orm

#endif // VIX_QUERY_BUILDER
