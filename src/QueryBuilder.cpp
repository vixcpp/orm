/**
 *
 *  @file QueryBuilder.cpp
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
#include <vix/orm/QueryBuilder.hpp>

#include <string>

namespace vix::orm::qb_internal
{
  /**
   * @brief Build a comma-separated placeholder list.
   *
   * Example:
   * - n = 0 -> ""
   * - n = 1 -> "?"
   * - n = 3 -> "?, ?, ?"
   *
   * This helper is intended for internal ORM query generation.
   *
   * @param n Number of placeholders to generate.
   * @return Placeholder list string.
   */
  std::string join_placeholders(std::size_t n)
  {
    if (n == 0)
    {
      return {};
    }

    std::string out;
    out.reserve(n * 3);

    for (std::size_t i = 0; i < n; ++i)
    {
      out.push_back('?');
      if (i + 1 < n)
      {
        out.push_back(',');
        out.push_back(' ');
      }
    }

    return out;
  }

} // namespace vix::orm::qb_internal
