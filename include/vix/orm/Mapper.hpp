/**
 *
 *  @file Mapper.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_MAPPER_HPP
#define VIX_MAPPER_HPP

#include <any>
#include <string>
#include <utility>
#include <vector>

#include <vix/db/core/Result.hpp>

namespace vix::orm
{
  /**
   * User-specialized mapper.
   *
   * Example:
   * template<>
   * struct Mapper<User> { ... };
   */
  template <class T>
  struct Mapper
  {
    static T fromRow(const vix::db::ResultRow &row);

    static std::vector<std::pair<std::string, std::any>>
    toInsertParams(const T &v);

    static std::vector<std::pair<std::string, std::any>>
    toUpdateParams(const T &v);
  };
} // namespace vix::orm

#endif // VIX_MAPPER_HPP
