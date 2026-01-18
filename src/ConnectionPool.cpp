/**
 *
 *  @file ConnectionPool.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/orm/ConnectionPool.hpp>

namespace vix::orm
{
  ConnectionPtr ConnectionPool::acquire()
  {
    std::unique_lock lk(m_);

    if (!idle_.empty())
    {
      auto c = idle_.front();
      idle_.pop();
      return c;
    }

    if (total_ < cfg_.max)
    {
      ++total_;
      lk.unlock();
      return factory_();
    }

    cv_.wait(lk, [&]
             { return !idle_.empty(); });

    auto c = idle_.front();
    idle_.pop();
    return c;
  }

  void ConnectionPool::release(ConnectionPtr c)
  {
    {
      std::lock_guard lk(m_);
      if (c)
        idle_.push(std::move(c));
    }
    cv_.notify_one();
  }

  void ConnectionPool::warmup()
  {
    std::lock_guard lk(m_);

    for (std::size_t i = 0; i < cfg_.min; ++i)
    {
      auto c = factory_();
      idle_.push(std::move(c));
      ++total_;
    }
  }

} // namespace vix::orm
