/**
 *
 *  @file db_compat.hpp
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
#ifndef VIX_ORM_DB_COMPAT_HPP
#define VIX_ORM_DB_COMPAT_HPP

#include <vix/db/core/Drivers.hpp>
#include <vix/db/core/Errors.hpp>
#include <vix/db/core/Value.hpp>

#include <vix/db/pool/ConnectionPool.hpp>
#include <vix/db/Transaction.hpp>
#include <vix/db/Database.hpp>
#include <vix/db/mig/Migration.hpp>
#include <vix/db/mig/FileMigrationsRunner.hpp>
#include <vix/db/mig/MigrationsRunner.hpp>
#include <vix/db/Sha256.hpp>

#if VIX_DB_HAS_MYSQL
#include <vix/db/drivers/mysql/MySQLDriver.hpp>
#endif

#include <any>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace vix::orm
{
  /**
   * @brief ORM compatibility layer for the vix::db module.
   *
   * This header re-exports commonly used DB-layer types into the
   * ORM namespace and provides small helpers used internally by
   * repositories, unit-of-work, and other ORM components.
   *
   * Its purpose is to keep ORM code concise while centralizing
   * DB interoperability logic in a single place.
   */
  using vix::db::Connection;
  using vix::db::ConnectionFactory;
  using vix::db::ConnectionPool;
  using vix::db::ConnectionPtr;
  using vix::db::Database;
  using vix::db::DBError;
  using vix::db::FileMigrationsRunner;
  using vix::db::Migration;
  using vix::db::MigrationsRunner;
  using vix::db::PoolConfig;
  using vix::db::PooledConn;
  using vix::db::ResultRow;
  using vix::db::ResultSet;
  using vix::db::Sha256;
  using vix::db::Statement;
  using vix::db::Transaction;

#if VIX_DB_HAS_MYSQL
  /// Convenience export for MySQL factory when MySQL support is enabled.
  using vix::db::make_mysql_factory;
#endif

  /**
   * @brief Represents a single ORM field/value pair.
   *
   * This alias is used across the ORM to describe values produced
   * by Mapper<T> specializations for INSERT and UPDATE operations.
   */
  using FieldValue = std::pair<std::string, std::any>;

  /**
   * @brief Represents a list of ORM field/value pairs.
   */
  using FieldValues = std::vector<FieldValue>;

  /**
   * @brief Convert a std::any value into a vix::db::DbValue.
   *
   * This function is used internally by the ORM when binding
   * mapper-produced values into prepared statements.
   *
   * Supported input types:
   * - empty std::any / std::nullptr_t
   * - vix::db::DbValue
   * - bool
   * - signed/unsigned integral types
   * - float / double
   * - std::string / std::string_view / const char* / char*
   * - vix::db::Blob
   *
   * @param value Input value stored as std::any.
   * @return Converted DbValue.
   *
   * @throws vix::db::DBError if the type is not supported.
   */
  inline vix::db::DbValue any_to_dbvalue_or_throw(const std::any &value)
  {
    using vix::db::Blob;
    using vix::db::DbValue;

    if (!value.has_value())
      return vix::db::null();

    if (value.type() == typeid(DbValue))
      return std::any_cast<DbValue>(value);

    if (value.type() == typeid(std::nullptr_t))
      return vix::db::null();

    if (value.type() == typeid(bool))
      return vix::db::b(std::any_cast<bool>(value));

    if (value.type() == typeid(int))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<int>(value)));
    if (value.type() == typeid(unsigned))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned>(value)));

    if (value.type() == typeid(long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<long>(value)));
    if (value.type() == typeid(unsigned long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned long>(value)));

    if (value.type() == typeid(long long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<long long>(value)));
    if (value.type() == typeid(unsigned long long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned long long>(value)));

    if (value.type() == typeid(std::size_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::size_t>(value)));

    if (value.type() == typeid(std::int64_t))
      return vix::db::i64(std::any_cast<std::int64_t>(value));
    if (value.type() == typeid(std::uint64_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint64_t>(value)));

    if (value.type() == typeid(std::int32_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int32_t>(value)));
    if (value.type() == typeid(std::uint32_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint32_t>(value)));

    if (value.type() == typeid(std::int16_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int16_t>(value)));
    if (value.type() == typeid(std::uint16_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint16_t>(value)));

    if (value.type() == typeid(std::int8_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int8_t>(value)));
    if (value.type() == typeid(std::uint8_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint8_t>(value)));

    if (value.type() == typeid(double))
      return vix::db::f64(std::any_cast<double>(value));
    if (value.type() == typeid(float))
      return vix::db::f64(static_cast<double>(std::any_cast<float>(value)));

    if (value.type() == typeid(std::string))
      return vix::db::str(std::any_cast<std::string>(value));

    if (value.type() == typeid(const std::string))
      return vix::db::str(std::any_cast<const std::string>(value));

    if (value.type() == typeid(std::string_view))
      return vix::db::str(std::string(std::any_cast<std::string_view>(value)));

    if (value.type() == typeid(const char *))
    {
      const char *s = std::any_cast<const char *>(value);
      return vix::db::str(std::string(s ? s : ""));
    }

    if (value.type() == typeid(char *))
    {
      char *s = std::any_cast<char *>(value);
      return vix::db::str(std::string(s ? s : ""));
    }

    if (value.type() == typeid(Blob))
      return DbValue{std::any_cast<Blob>(value)};

    throw vix::db::DBError(
        std::string("ORM: unsupported std::any type: ") + value.type().name());
  }

  /**
   * @brief Convert a field value directly into a DbValue.
   *
   * @param field ORM field/value pair.
   * @return Converted DbValue.
   */
  inline vix::db::DbValue field_to_dbvalue_or_throw(const FieldValue &field)
  {
    return any_to_dbvalue_or_throw(field.second);
  }

  /**
   * @brief Bind a list of ORM fields to a prepared statement.
   *
   * Fields are bound sequentially starting at @p start_index.
   *
   * @param st Prepared statement.
   * @param fields Field/value pairs to bind.
   * @param start_index One-based bind index.
   */
  inline void bind_field_values(vix::db::Statement &st,
                                const FieldValues &fields,
                                std::size_t start_index = 1)
  {
    std::size_t index = start_index;
    for (const auto &field : fields)
    {
      st.bind(index++, field_to_dbvalue_or_throw(field));
    }
  }

  /**
   * @brief Return true if the field list is empty.
   *
   * @param fields Field/value list.
   * @return true if empty, false otherwise.
   */
  inline bool field_values_empty(const FieldValues &fields) noexcept
  {
    return fields.empty();
  }

  /**
   * @brief Validate that a field list is not empty.
   *
   * @param fields Field/value list.
   * @param context Short context string used in the error message.
   *
   * @throws vix::db::DBError if the field list is empty.
   */
  inline void require_non_empty_fields(const FieldValues &fields,
                                       std::string_view context)
  {
    if (fields.empty())
    {
      throw vix::db::DBError(
          std::string("ORM: empty field list in ") + std::string(context));
    }
  }

} // namespace vix::orm

#endif // VIX_ORM_DB_COMPAT_HPP
