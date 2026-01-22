/**
 *
 *  @file db_compat.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
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
#include <vix/db/drivers/mysql/MySQLDriver.hpp>

#include <any>
#include <cstdint>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

namespace vix::orm
{
  using vix::db::Connection;
  using vix::db::ConnectionFactory;
  using vix::db::ConnectionPool;
  using vix::db::ConnectionPtr;
  using vix::db::Database;
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

  using vix::db::DBError;
  using vix::db::make_mysql_factory;

  inline vix::db::DbValue any_to_dbvalue_or_throw(const std::any &a)
  {
    using vix::db::Blob;
    using vix::db::DbValue;

    if (!a.has_value())
      return vix::db::null();

    if (a.type() == typeid(DbValue))
      return std::any_cast<DbValue>(a);

    // nullptr explicit
    if (a.type() == typeid(std::nullptr_t))
      return vix::db::null();

    // bool
    if (a.type() == typeid(bool))
      return vix::db::b(std::any_cast<bool>(a));

    if (a.type() == typeid(int))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<int>(a)));
    if (a.type() == typeid(unsigned))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned>(a)));

    if (a.type() == typeid(long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<long>(a)));
    if (a.type() == typeid(unsigned long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned long>(a)));

    if (a.type() == typeid(long long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<long long>(a)));
    if (a.type() == typeid(unsigned long long))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<unsigned long long>(a)));

    if (a.type() == typeid(std::size_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::size_t>(a)));

    if (a.type() == typeid(std::int64_t))
      return vix::db::i64(std::any_cast<std::int64_t>(a));
    if (a.type() == typeid(std::uint64_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint64_t>(a)));

    if (a.type() == typeid(std::int32_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int32_t>(a)));
    if (a.type() == typeid(std::uint32_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint32_t>(a)));

    if (a.type() == typeid(std::int16_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int16_t>(a)));
    if (a.type() == typeid(std::uint16_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint16_t>(a)));

    if (a.type() == typeid(std::int8_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::int8_t>(a)));
    if (a.type() == typeid(std::uint8_t))
      return vix::db::i64(static_cast<std::int64_t>(std::any_cast<std::uint8_t>(a)));

    if (a.type() == typeid(double))
      return vix::db::f64(std::any_cast<double>(a));
    if (a.type() == typeid(float))
      return vix::db::f64(static_cast<double>(std::any_cast<float>(a)));

    if (a.type() == typeid(std::string))
      return vix::db::str(std::any_cast<std::string>(a));

    if (a.type() == typeid(const std::string))
      return vix::db::str(std::any_cast<const std::string>(a));

    if (a.type() == typeid(std::string_view))
      return vix::db::str(std::string(std::any_cast<std::string_view>(a)));

    if (a.type() == typeid(const char *))
    {
      const char *s = std::any_cast<const char *>(a);
      return vix::db::str(std::string(s ? s : ""));
    }

    if (a.type() == typeid(char *))
    {
      char *s = std::any_cast<char *>(a);
      return vix::db::str(std::string(s ? s : ""));
    }

    if (a.type() == typeid(Blob))
      return DbValue{std::any_cast<Blob>(a)};

    throw vix::db::DBError(std::string{"ORM: unsupported std::any type: "} + a.type().name());
  }

} // namespace vix::orm

#endif // VIX_ORM_DB_COMPAT_HPP
