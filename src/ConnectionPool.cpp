#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/MySQLDriver.hpp> // provides MySQLConnection + make_mysql_conn

#include <utility>

/**
 * @file ConnectionPool.cpp
 * @brief Implementation details for the Vix ORM connection pooling system.
 *
 * This file provides the internal logic for acquiring and releasing
 * database connections from a shared pool.
 *
 * Current backend: MySQL (internals are here to keep the header driver-agnostic).
 */

namespace Vix::orm
{
    std::shared_ptr<Connection> ConnectionPool::acquire()
    {
        std::unique_lock lk(m_);

        // 1) Reuse an idle connection if available
        if (!idle_.empty())
        {
            auto c = idle_.front();
            idle_.pop();
            return c;
        }

        // 2) Create a new one if under max
        if (total_ < cfg_.max)
        {
            ++total_;
            lk.unlock(); // don't hold the lock while doing I/O

            // Create a concrete MySQL connection then upcast to Connection
            auto raw = make_mysql_conn(host_, user_, pass_, db_);
            auto mysql_conn = std::make_shared<MySQLConnection>(std::move(raw));
            return std::static_pointer_cast<Connection>(mysql_conn);
        }

        // 3) Otherwise, wait for a release
        cv_.wait(lk, [&]
                 { return !idle_.empty(); });

        auto c = idle_.front();
        idle_.pop();
        return c;
    }

    void ConnectionPool::release(std::shared_ptr<Connection> c)
    {
        {
            std::lock_guard lk(m_);
            if (c)
                idle_.push(std::move(c));
        }
        cv_.notify_one();
    }

} // namespace Vix::orm
