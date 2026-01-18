/**
 *
 *  @file ConnectionPool.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_ORM_CONNECTION_POOL_HPP
#define VIX_ORM_CONNECTION_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

#include <vix/orm/Drivers.hpp>

namespace vix::orm
{
  struct PoolConfig
  {
    std::size_t min = 1;
    std::size_t max = 8;
  };

  class ConnectionPool
  {
    ConnectionFactory factory_;
    PoolConfig cfg_{};

    std::mutex m_;
    std::condition_variable cv_;
    std::queue<ConnectionPtr> idle_;
    std::size_t total_ = 0;

  public:
    ConnectionPool(ConnectionFactory factory, PoolConfig cfg = {})
        : factory_(std::move(factory)), cfg_(cfg) {}

    ConnectionPtr acquire();
    void release(ConnectionPtr c);
    void warmup();
  };

  class PooledConn final
  {
    ConnectionPool &pool_;
    ConnectionPtr c_;

  public:
    explicit PooledConn(ConnectionPool &p)
        : pool_(p), c_(p.acquire()) {}

    ~PooledConn()
    {
      if (c_)
        pool_.release(std::move(c_));
    }

    PooledConn(const PooledConn &) = delete;
    PooledConn &operator=(const PooledConn &) = delete;

    Connection &get() { return *c_; }
    ConnectionPtr &ptr() { return c_; }
    const ConnectionPtr &ptr() const { return c_; }
  };

} // namespace vix::orm

#endif // VIX_ORM_CONNECTION_POOL_HPP
