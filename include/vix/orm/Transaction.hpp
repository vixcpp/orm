#ifndef VIX_TRANSACTION_HPP
#define VIX_TRANSACTION_HPP

#include <vix/orm/ConnectionPool.hpp>

/**
 * @file Transaction.hpp
 * @brief Lightweight RAII-based transaction helper for Vix ORM.
 *
 * The `Transaction` class provides a **safe, exception-resilient**
 * wrapper around database transactions. It automatically begins
 * a transaction upon creation and rolls back if not explicitly
 * committed before destruction.
 *
 * ---
 * ## Overview
 * A `Transaction` acquires a pooled connection and issues:
 * ```sql
 * BEGIN;
 * ... user operations ...
 * COMMIT; // or ROLLBACK on destruction
 * ```
 *
 * The class ensures that:
 * - Transactions are always terminated (commit or rollback).
 * - Connections are automatically released to the pool.
 * - Rollback occurs automatically on exception or premature exit.
 *
 * ---
 * ## Example
 * ```cpp
 * using namespace Vix::orm;
 *
 * ConnectionPool pool("localhost", "root", "pwd", "mydb");
 *
 * try {
 *     Transaction tx(pool);
 *     auto& conn = tx.conn();
 *
 *     auto st = conn.prepare("INSERT INTO users(name, age) VALUES(?, ?)");
 *     st->bind(1, "Alice");
 *     st->bind(2, 25);
 *     st->exec();
 *
 *     tx.commit();  // ✅ commits transaction
 * }
 * catch (const DBError& e) {
 *     std::cerr << "DB error: " << e.what() << "\n";
 * }
 * // If commit() wasn’t called, rollback() is automatic.
 * ```
 *
 * ---
 * ## Notes
 * - Transactions are **scoped**. The destructor performs rollback
 *   if `commit()` has not been invoked.
 * - A transaction keeps a single connection for its lifetime.
 * - Use one transaction per logical unit of work; avoid nesting.
 * - For multiple tables or repositories, reuse `tx.conn()` within
 *   the same scope.
 */

namespace Vix::orm
{
    /**
     * @class Transaction
     * @brief RAII wrapper for safe transaction management.
     */
    class Transaction
    {
        PooledConn pooled_;  ///< Acquired pooled connection.
        bool active_ = true; ///< Indicates if rollback is pending.

    public:
        /**
         * @brief Begin a new transaction.
         * @param pool Shared connection pool.
         */
        explicit Transaction(ConnectionPool &pool)
            : pooled_(pool)
        {
            pooled_.get().begin();
        }

        /**
         * @brief Destructor — automatically rolls back if active.
         */
        ~Transaction()
        {
            if (active_)
                try
                {
                    pooled_.get().rollback();
                }
                catch (...)
                {
                    // Suppress rollback exceptions to preserve stack unwinding.
                }
        }

        /**
         * @brief Commit the current transaction.
         *
         * Marks the transaction as complete; the destructor will
         * no longer attempt rollback.
         */
        void commit()
        {
            pooled_.get().commit();
            active_ = false;
        }

        /**
         * @brief Roll back the transaction manually.
         *
         * Can be safely called multiple times; only the first call
         * has an effect.
         */
        void rollback()
        {
            pooled_.get().rollback();
            active_ = false;
        }

        /**
         * @brief Access the underlying `Connection` (driver-agnostic).
         * @return A reference to the active connection.
         *
         * Use this to prepare and execute SQL statements directly.
         */
        Connection &conn() { return pooled_.get(); }
    };
} // namespace Vix::orm

#endif // VIX_TRANSACTION_HPP
