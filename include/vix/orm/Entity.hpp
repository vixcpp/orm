/**
 *
 *  @file Entity.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_ENTITY_HPP
#define VIX_ENTITY_HPP

#include <cstdint>
#include <string>

namespace vix::orm
{
  struct Entity
  {
    virtual ~Entity() = default;
  };
} // namespace Vix::orm

#endif // VIX_ENTITY_HPP
