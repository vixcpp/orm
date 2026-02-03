/**
 *
 *  @file Entity.hpp
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
#ifndef VIX_ENTITY_HPP
#define VIX_ENTITY_HPP

#include <cstdint>
#include <string>

namespace vix::orm
{
  /**
   * @brief Base class for ORM entities.
   *
   * Entity is the common base type for all ORM-managed objects.
   * It provides a polymorphic interface and a virtual destructor,
   * allowing entities to be handled via base pointers or references.
   *
   * This type intentionally contains no data or behavior and serves
   * purely as a semantic marker and extension point for the ORM.
   */
  struct Entity
  {
    virtual ~Entity() = default;
  };

} // namespace vix::orm

#endif // VIX_ENTITY_HPP
