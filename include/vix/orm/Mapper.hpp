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
#include <utility>
#include <vector>

#include <vix/db/core/Result.hpp>

namespace vix::orm
{
  /**
   * @brief User-specialized mapper for ORM entities.
   *
   * Mapper<T> defines how an ORM entity of type @p T is:
   * - constructed from a database result row
   * - converted into parameters for INSERT statements
   * - converted into parameters for UPDATE statements
   *
   * This template is intended to be fully specialized by the user
   * for each entity type.
   *
   * Example:
   * @code
   * template<>
   * struct Mapper<User>
   * {
   *   static User fromRow(const vix::db::ResultRow &row);
   *
   *   static std::vector<std::pair<std::string, std::any>>
   *   toInsertParams(const User &u);
   *
   *   static std::vector<std::pair<std::string, std::any>>
   *   toUpdateParams(const User &u);
   * };
   * @endcode
   */
  template <class T>
  struct Mapper
  {
    /**
     * @brief Construct an entity instance from a database row.
     *
     * This method is called when materializing query results
     * into ORM entities.
     *
     * @param row Database result row.
     * @return Constructed entity instance.
     */
    static T fromRow(const vix::db::ResultRow &row);

    /**
     * @brief Produce parameters for an INSERT statement.
     *
     * Returns a list of column/value pairs representing the
     * fields to be inserted. Values are stored as std::any
     * and later converted to DbValue by the ORM.
     *
     * @param v Entity instance.
     * @return List of column/value pairs.
     */
    static std::vector<std::pair<std::string, std::any>>
    toInsertParams(const T &v);

    /**
     * @brief Produce parameters for an UPDATE statement.
     *
     * Returns a list of column/value pairs representing the
     * fields to be updated. Typically excludes immutable
     * fields such as primary keys.
     *
     * @param v Entity instance.
     * @return List of column/value pairs.
     */
    static std::vector<std::pair<std::string, std::any>>
    toUpdateParams(const T &v);
  };

} // namespace vix::orm

#endif // VIX_MAPPER_HPP
