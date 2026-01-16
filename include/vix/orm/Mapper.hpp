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

#include <vix/orm/Drivers.hpp>
#include <string>
#include <vector>
#include <any>

namespace vix::orm
{
  template <class T>
  struct Mapper
  {
    static T fromRow(const ResultRow &row);
    static std::vector<std::pair<std::string, std::any>> toInsertParams(const T &v);
    static std::vector<std::pair<std::string, std::any>> toUpdateParams(const T &v);
  };
} // namespace Vix::orm

#endif // VIX_MAPPER_HPP
