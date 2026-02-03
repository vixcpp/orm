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

#include <vix/orm/db_compat.hpp>

namespace vix::orm
{
  /**
   * @brief Transactional unit of work.
   *
   * UnitOfWork groups multiple repository or direct SQL operations
   * into a single transactional scope.
   *
   * A database transaction is started on construction and is
   * automatically rolled back on destruction unless explicitly
   * committed.
   *
   * This class follows RAII semantics and is intended to be used
   * as a short-lived scope object.
   */
  class UnitOfWork
  {
    vix::db::Transaction tx_;

  public:
    /**
     * @brief Begin a new unit of work.
     *
     * Acquires a connection from the pool and starts a transaction.
     *
     * @param pool Connection pool.
     */
    explicit UnitOfWork(vix::db::ConnectionPool &pool)
        : tx_(pool) {}

    UnitOfWork(const UnitOfWork &) = delete;
    UnitOfWork &operator=(const UnitOfWork &) = delete;

    UnitOfWork(UnitOfWork &&) noexcept = default;
    UnitOfWork &operator=(UnitOfWork &&) noexcept = default;

    /**
     * @brief Commit the transaction.
     *
     * After calling commit(), the UnitOfWork becomes inactive
     * and will not perform a rollback on destruction.
     */
    void commit() { tx_.commit(); }

    /**
     * @brief Roll back the transaction explicitly.
     *
     * After calling rollback(), the UnitOfWork becomes inactive.
     */
    void rollback() { tx_.rollback(); }

    /**
     * @brief Access the underlying database connection.
     *
     * Intended for repositories or low-level queries that need
     * to participate in the same transaction.
     *
     * @return Reference to an active database connection.
     */
    vix::db::Connection &conn() { return tx_.conn(); }

    /**
     * @brief Const access to the underlying database connection.
     */
    const vix::db::Connection &conn() const
    {
      return const_cast<UnitOfWork *>(this)->tx_.conn();
    }
  };

} // namespace vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
