/**
 *
 *  @file UnitOfWork.hpp
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
#ifndef VIX_UNIT_OF_WORK_HPP
#define VIX_UNIT_OF_WORK_HPP

#include <utility>

#include <vix/orm/db_compat.hpp>

namespace vix::orm
{
  /**
   * @brief Transactional unit of work.
   *
   * UnitOfWork groups multiple ORM or direct SQL operations into a
   * single transactional scope.
   *
   * A transaction is started on construction and is automatically
   * rolled back on destruction unless explicitly committed or rolled back.
   *
   * Design goals:
   * - explicit transaction boundaries
   * - RAII safety
   * - no hidden persistence
   * - simple integration with repositories
   */
  class UnitOfWork
  {
    vix::db::ConnectionPool *pool_ = nullptr;
    vix::db::Transaction tx_;
    bool active_ = true;

  public:
    /**
     * @brief Begin a new unit of work from a connection pool.
     *
     * @param pool Connection pool.
     */
    explicit UnitOfWork(vix::db::ConnectionPool &pool)
        : pool_(&pool), tx_(pool)
    {
    }

    /**
     * @brief Begin a new unit of work from a Database facade.
     *
     * @param db Database facade.
     */
    explicit UnitOfWork(vix::db::Database &db)
        : UnitOfWork(db.pool())
    {
    }

    ~UnitOfWork() = default;

    UnitOfWork(const UnitOfWork &) = delete;
    UnitOfWork &operator=(const UnitOfWork &) = delete;

    UnitOfWork(UnitOfWork &&other) noexcept
        : pool_(other.pool_), tx_(std::move(other.tx_)), active_(other.active_)
    {
      other.pool_ = nullptr;
      other.active_ = false;
    }

    UnitOfWork &operator=(UnitOfWork &&) = delete;

    /**
     * @brief Return whether the unit of work is still active.
     *
     * A unit of work becomes inactive after commit() or rollback().
     *
     * @return true if active, false otherwise.
     */
    [[nodiscard]] bool active() const noexcept
    {
      return active_;
    }

    /**
     * @brief Commit the transaction.
     *
     * After commit, the unit of work becomes inactive.
     */
    void commit()
    {
      if (!active_)
      {
        return;
      }

      tx_.commit();
      active_ = false;
    }

    /**
     * @brief Roll back the transaction explicitly.
     *
     * After rollback, the unit of work becomes inactive.
     */
    void rollback()
    {
      if (!active_)
      {
        return;
      }

      tx_.rollback();
      active_ = false;
    }

    /**
     * @brief Access the underlying database connection.
     *
     * This allows repositories and explicit SQL statements to run in
     * the same transactional scope.
     *
     * @return Active database connection.
     */
    vix::db::Connection &conn()
    {
      return tx_.conn();
    }

    /**
     * @brief Const access to the underlying database connection.
     *
     * @return Active database connection.
     */
    const vix::db::Connection &conn() const
    {
      return const_cast<UnitOfWork *>(this)->tx_.conn();
    }

    /**
     * @brief Access the connection pool used by this unit of work.
     *
     * @return Connection pool reference.
     */
    vix::db::ConnectionPool &pool()
    {
      return *pool_;
    }

    /**
     * @brief Const access to the connection pool used by this unit of work.
     *
     * @return Connection pool reference.
     */
    const vix::db::ConnectionPool &pool() const
    {
      return *pool_;
    }

    /**
     * @brief Access the underlying transaction object.
     *
     * This is intended for advanced internal integration scenarios.
     *
     * @return Transaction reference.
     */
    vix::db::Transaction &transaction()
    {
      return tx_;
    }

    /**
     * @brief Const access to the underlying transaction object.
     *
     * @return Transaction reference.
     */
    const vix::db::Transaction &transaction() const
    {
      return tx_;
    }
  };

} // namespace vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
