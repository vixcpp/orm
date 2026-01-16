/**
 *
 *  @file QueryBuilder.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/orm/QueryBuilder.hpp>

#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>
#include <any>

namespace vix::orm::qb_internal
{
  inline std::string join_placeholders(std::size_t n)
  {
    if (n == 0)
      return {};
    std::string s;
    s.reserve(n * 3); // "?, " * (n-1) + "?"
    for (std::size_t i = 0; i < n; ++i)
    {
      s.push_back('?');
      if (i + 1 < n)
      {
        s.push_back(',');
        s.push_back(' ');
      }
    }
    return s;
  }

  template <class Range>
  inline QueryBuilder &append_in_list(QueryBuilder &qb, const Range &values)
  {
    using std::begin;
    using std::end;
    const auto n = static_cast<std::size_t>(std::distance(begin(values), end(values)));
    qb.raw("IN (").raw(join_placeholders(n)).raw(")");
    for (const auto &v : values)
    {
      qb.param(std::any{v});
    }
    return qb;
  }

} // namespace Vix::orm::qb_internal
