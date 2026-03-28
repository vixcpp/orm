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

namespace vix::orm
{
  /**
   * @brief Base class for ORM entities.
   *
   * Entity represents a domain object managed by the ORM.
   *
   * Design principles:
   * - No hidden behavior
   * - No automatic persistence
   * - No runtime reflection
   * - Explicit control by the developer
   *
   * This class provides a minimal identity model used internally
   * by repositories and UnitOfWork without imposing any constraints
   * on user-defined types.
   *
   * Users are free to ignore inheritance entirely, but extending
   * Entity enables advanced ORM features such as tracking and identity.
   */
  struct Entity
  {
    /**
     * @brief Virtual destructor for polymorphic usage.
     */
    virtual ~Entity() = default;

    /**
     * @brief Return the entity identifier.
     *
     * This method is optional and may be overridden by derived types.
     *
     * @return Identifier value.
     */
    virtual std::int64_t id() const noexcept { return 0; }

    /**
     * @brief Set the entity identifier.
     *
     * This method is optional and may be overridden by derived types.
     *
     * @param value Identifier value.
     */
    virtual void setId(std::int64_t) noexcept {}
  };

} // namespace vix::orm

#endif // VIX_ENTITY_HPP
