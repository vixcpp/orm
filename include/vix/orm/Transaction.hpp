/**
 *
 *  @file Transaction.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_TRANSACTION_HPP
#define VIX_TRANSACTION_HPP

#include <vix/orm/ConnectionPool.hpp>

namespace vix::orm
{
  class Transaction
  {
    PooledConn pooled_;
    bool active_ = true;

  public:
    explicit Transaction(ConnectionPool &pool)
        : pooled_(pool)
    {
      pooled_.get().begin();
    }

    ~Transaction()
    {
      if (active_)
        try
        {
          pooled_.get().rollback();
        }
        catch (...)
        {
        }
    }

    void commit()
    {
      pooled_.get().commit();
      active_ = false;
    }

    void rollback()
    {
      pooled_.get().rollback();
      active_ = false;
    }

    Connection &conn() { return pooled_.get(); }
  };
} // namespace Vix::orm

#endif // VIX_TRANSACTION_HPP
