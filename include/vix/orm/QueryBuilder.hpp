/**
 *
 *  @file QueryBuilder.hpp
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
#ifndef VIX_QUERY_BUILDER
#define VIX_QUERY_BUILDER

#include <string>
#include <string_view>
#include <vector>

#include <vix/db/core/Value.hpp>

namespace vix::orm
{
  /**
   * @brief Lightweight SQL query builder with bound parameters.
   *
   * QueryBuilder helps construct SQL statements incrementally while
   * keeping parameters separated from the SQL string. Parameters are
   * collected as DbValue instances and are intended to be bound to a
   * prepared statement by the ORM or DB layer.
   *
   * This class does not attempt to validate SQL syntax and intentionally
   * avoids abstraction leakage. It is designed to be minimal, explicit,
   * and composable.
   */
  class QueryBuilder
  {
    std::string sql_;
    std::vector<vix::db::DbValue> params_;

  public:
    /**
     * @brief Append raw SQL text.
     *
     * @param s SQL fragment.
     * @return Reference to this builder.
     */
    QueryBuilder &raw(std::string_view s)
    {
      sql_.append(s);
      return *this;
    }

    /**
     * @brief Append a single space character.
     *
     * Convenience helper for readable chaining.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &space()
    {
      sql_.push_back(' ');
      return *this;
    }

    /**
     * @brief Append a parameter value.
     *
     * The parameter is stored and expected to be bound to the
     * next positional placeholder by the caller.
     *
     * @param v Database value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(const vix::db::DbValue &v)
    {
      params_.push_back(v);
      return *this;
    }

    /**
     * @brief Append a parameter value (move).
     *
     * @param v Database value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(vix::db::DbValue &&v)
    {
      params_.push_back(std::move(v));
      return *this;
    }

    /// Convenience overloads for common C++ types
    QueryBuilder &param(std::int64_t v) { return param(vix::db::i64(v)); }
    QueryBuilder &param(int v) { return param(vix::db::i64(static_cast<std::int64_t>(v))); }
    QueryBuilder &param(std::uint64_t v) { return param(vix::db::i64(static_cast<std::int64_t>(v))); }
    QueryBuilder &param(double v) { return param(vix::db::f64(v)); }
    QueryBuilder &param(bool v) { return param(vix::db::b(v)); }

    QueryBuilder &param(std::string v) { return param(vix::db::str(std::move(v))); }
    QueryBuilder &param(const char *v) { return param(vix::db::str(std::string(v ? v : ""))); }

    /**
     * @brief Append a NULL parameter.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &paramNull() { return param(vix::db::null()); }

    /**
     * @brief Access the constructed SQL string.
     *
     * @return SQL string.
     */
    const std::string &sql() const { return sql_; }

    /**
     * @brief Access the collected parameters.
     *
     * @return Vector of bound parameters.
     */
    const std::vector<vix::db::DbValue> &params() const { return params_; }
  };

} // namespace vix::orm

#endif // VIX_QUERY_BUILDER
