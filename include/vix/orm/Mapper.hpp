/**
 *
 *  @file Mapper.hpp
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
#ifndef VIX_MAPPER_HPP
#define VIX_MAPPER_HPP

#include <any>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/db/core/Result.hpp>

namespace vix::orm
{
  /**
   * @brief Represents a single column/value pair used by ORM mappers.
   *
   * This type is used internally by repositories to build INSERT and
   * UPDATE statements from user-defined entities.
   *
   * Values are stored as std::any at the mapper boundary to keep the
   * specialization interface flexible and independent from low-level
   * database driver details.
   */
  using FieldValue = std::pair<std::string, std::any>;

  /**
   * @brief Represents a list of ORM field/value pairs.
   */
  using FieldValues = std::vector<FieldValue>;

  /**
   * @brief User-specialized mapper for ORM entities.
   *
   * Mapper<T> defines how an entity of type @p T is:
   * - materialized from a database row
   * - converted into fields for INSERT statements
   * - converted into fields for UPDATE statements
   *
   * This template is intended to be fully specialized by the user for
   * each entity type.
   *
   * Example:
   * @code
   * template<>
   * struct Mapper<User>
   * {
   *   static User fromRow(const vix::db::ResultRow &row);
   *
   *   static vix::orm::FieldValues toInsertFields(const User &u);
   *
   *   static vix::orm::FieldValues toUpdateFields(const User &u);
   * };
   * @endcode
   *
   * Design goals:
   * - explicit mapping
   * - no reflection
   * - no hidden metadata
   * - no runtime magic
   */
  template <class T>
  struct Mapper
  {
    static_assert(!std::is_reference_v<T>,
                  "Mapper<T> cannot be specialized for reference types");

    /**
     * @brief Construct an entity instance from a database row.
     *
     * This method is used when materializing query results into
     * user-defined entity objects.
     *
     * @param row Database result row.
     * @return Constructed entity instance.
     */
    static T fromRow(const vix::db::ResultRow &row);

    /**
     * @brief Produce field/value pairs for an INSERT statement.
     *
     * The returned fields define which columns are inserted and which
     * values are bound for the operation.
     *
     * @param value Entity instance.
     * @return Field/value pairs for insertion.
     */
    static FieldValues toInsertFields(const T &value);

    /**
     * @brief Produce field/value pairs for an UPDATE statement.
     *
     * The returned fields define which columns are updated and which
     * values are bound for the operation.
     *
     * Primary keys and immutable fields are typically excluded.
     *
     * @param value Entity instance.
     * @return Field/value pairs for update.
     */
    static FieldValues toUpdateFields(const T &value);

    /**
     * @brief Backward-compatible alias for older mapper implementations.
     *
     * Existing code using toInsertParams can continue to compile while
     * the ORM migrates toward the clearer field-based naming.
     *
     * @param value Entity instance.
     * @return Field/value pairs for insertion.
     */
    static FieldValues toInsertParams(const T &value)
    {
      return toInsertFields(value);
    }

    /**
     * @brief Backward-compatible alias for older mapper implementations.
     *
     * Existing code using toUpdateParams can continue to compile while
     * the ORM migrates toward the clearer field-based naming.
     *
     * @param value Entity instance.
     * @return Field/value pairs for update.
     */
    static FieldValues toUpdateParams(const T &value)
    {
      return toUpdateFields(value);
    }
  };

} // namespace vix::orm

#endif // VIX_MAPPER_HPP
