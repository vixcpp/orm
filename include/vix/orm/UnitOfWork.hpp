/**
 *
 *  @file UnitOfWork.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_UNIT_OF_WORK_HPP
#define VIX_UNIT_OF_WORK_HPP

#include <vix/orm/db_compat.hpp>

namespace vix::orm
{
  class UnitOfWork
  {
    vix::db::Transaction tx_;

  public:
    explicit UnitOfWork(vix::db::ConnectionPool &pool)
        : tx_(pool) {}

    UnitOfWork(const UnitOfWork &) = delete;
    UnitOfWork &operator=(const UnitOfWork &) = delete;

    UnitOfWork(UnitOfWork &&) noexcept = default;
    UnitOfWork &operator=(UnitOfWork &&) noexcept = default;

    void commit() { tx_.commit(); }
    void rollback() { tx_.rollback(); }

    vix::db::Connection &conn() { return tx_.conn(); }
    const vix::db::Connection &conn() const { return const_cast<UnitOfWork *>(this)->tx_.conn(); }
  };
} // namespace vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
