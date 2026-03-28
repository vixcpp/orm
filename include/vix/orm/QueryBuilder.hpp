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
#ifndef VIX_ORM_QUERY_BUILDER_HPP
#define VIX_ORM_QUERY_BUILDER_HPP

#include <vix/orm/db_compat.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace vix::orm
{
  /**
   * @brief Lightweight SQL query builder with bound parameters.
   *
   * QueryBuilder helps construct SQL statements incrementally while
   * keeping parameters separate from the SQL string.
   *
   * Design goals:
   * - explicit SQL
   * - predictable behavior
   * - no hidden query generation
   * - simple parameter collection
   *
   * This is not a full ORM query DSL. It is a small utility for building
   * prepared statements cleanly while staying close to SQL.
   */
  class QueryBuilder
  {
    std::string sql_;
    std::vector<vix::db::DbValue> params_;

  public:
    /**
     * @brief Construct an empty query builder.
     */
    QueryBuilder() = default;

    /**
     * @brief Construct a query builder from an initial SQL fragment.
     *
     * @param sql Initial SQL text.
     */
    explicit QueryBuilder(std::string_view sql)
    {
      sql_.append(sql);
    }

    /**
     * @brief Reserve memory for SQL and parameter storage.
     *
     * @param sql_capacity Expected SQL size.
     * @param param_capacity Expected number of parameters.
     * @return Reference to this builder.
     */
    QueryBuilder &reserve(std::size_t sql_capacity,
                          std::size_t param_capacity = 0)
    {
      sql_.reserve(sql_capacity);
      params_.reserve(param_capacity);
      return *this;
    }

    /**
     * @brief Remove all accumulated SQL and parameters.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &clear()
    {
      sql_.clear();
      params_.clear();
      return *this;
    }

    /**
     * @brief Return whether both SQL and params are empty.
     *
     * @return true if empty, false otherwise.
     */
    bool empty() const noexcept
    {
      return sql_.empty() && params_.empty();
    }

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
     * @brief Append raw SQL text followed by a space.
     *
     * @param s SQL fragment.
     * @return Reference to this builder.
     */
    QueryBuilder &rawSpace(std::string_view s)
    {
      sql_.append(s);
      sql_.push_back(' ');
      return *this;
    }

    /**
     * @brief Append a single space character.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &space()
    {
      sql_.push_back(' ');
      return *this;
    }

    /**
     * @brief Append a newline character.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &newline()
    {
      sql_.push_back('\n');
      return *this;
    }

    /**
     * @brief Append a parameter value.
     *
     * @param value Database value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(const vix::db::DbValue &value)
    {
      params_.push_back(value);
      return *this;
    }

    /**
     * @brief Append a parameter value by move.
     *
     * @param value Database value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(vix::db::DbValue &&value)
    {
      params_.push_back(std::move(value));
      return *this;
    }

    /**
     * @brief Append a signed 64-bit integer parameter.
     *
     * @param value Integer value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(std::int64_t value)
    {
      return param(vix::db::i64(value));
    }

    /**
     * @brief Append a signed integer parameter.
     *
     * @param value Integer value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(int value)
    {
      return param(vix::db::i64(static_cast<std::int64_t>(value)));
    }

    /**
     * @brief Append an unsigned 64-bit integer parameter.
     *
     * @param value Integer value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(std::uint64_t value)
    {
      return param(vix::db::i64(static_cast<std::int64_t>(value)));
    }

    /**
     * @brief Append a floating-point parameter.
     *
     * @param value Double value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(double value)
    {
      return param(vix::db::f64(value));
    }

    /**
     * @brief Append a boolean parameter.
     *
     * @param value Boolean value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(bool value)
    {
      return param(vix::db::b(value));
    }

    /**
     * @brief Append a string parameter.
     *
     * @param value String value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(std::string value)
    {
      return param(vix::db::str(std::move(value)));
    }

    /**
     * @brief Append a string_view parameter.
     *
     * @param value String view.
     * @return Reference to this builder.
     */
    QueryBuilder &param(std::string_view value)
    {
      return param(vix::db::str(std::string(value)));
    }

    /**
     * @brief Append a C-string parameter.
     *
     * @param value C-string pointer.
     * @return Reference to this builder.
     */
    QueryBuilder &param(const char *value)
    {
      return param(vix::db::str(std::string(value ? value : "")));
    }

    /**
     * @brief Append a blob parameter.
     *
     * @param value Blob value.
     * @return Reference to this builder.
     */
    QueryBuilder &param(vix::db::Blob value)
    {
      return param(vix::db::DbValue{std::move(value)});
    }

    /**
     * @brief Append a NULL parameter.
     *
     * @return Reference to this builder.
     */
    QueryBuilder &paramNull()
    {
      return param(vix::db::null());
    }

    /**
     * @brief Bind all collected parameters to a prepared statement.
     *
     * Binding starts at index 1.
     *
     * @param st Prepared statement.
     */
    void bind(vix::db::Statement &st) const
    {
      for (std::size_t i = 0; i < params_.size(); ++i)
      {
        st.bind(i + 1, params_[i]);
      }
    }

    /**
     * @brief Access the constructed SQL string.
     *
     * @return SQL string.
     */
    const std::string &sql() const noexcept
    {
      return sql_;
    }

    /**
     * @brief Access the collected parameters.
     *
     * @return Parameter list.
     */
    const std::vector<vix::db::DbValue> &params() const noexcept
    {
      return params_;
    }

    /**
     * @brief Move out the SQL string.
     *
     * @return SQL string.
     */
    std::string takeSql() noexcept
    {
      return std::move(sql_);
    }

    /**
     * @brief Move out the parameter list.
     *
     * @return Parameter list.
     */
    std::vector<vix::db::DbValue> takeParams() noexcept
    {
      return std::move(params_);
    }
  };

} // namespace vix::orm

#endif // VIX_ORM_QUERY_BUILDER_HPP
