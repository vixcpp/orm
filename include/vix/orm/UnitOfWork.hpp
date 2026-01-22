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

#include <vix/db/Transaction.hpp>

namespace vix::orm
{
  class UnitOfWork
  {
    vix::db::Transaction tx_;

  public:
    explicit UnitOfWork(vix::db::ConnectionPool &pool) : tx_(pool) {}
    void commit() { tx_.commit(); }
    void rollback() { tx_.rollback(); }
    vix::db::Connection &conn() { return tx_.conn(); }
  };
} // namespace Vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
