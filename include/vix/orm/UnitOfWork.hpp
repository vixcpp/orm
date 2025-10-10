#ifndef VIX_UNIT_OF_WORK_HPP
#define VIX_UNIT_OF_WORK_HPP

#include <vix/orm/Transaction.hpp>

/**
 * @file UnitOfWork.hpp
 * @brief Higher-level transactional context that encapsulates a Transaction.
 *
 * The **UnitOfWork** class provides a convenient abstraction for grouping
 * multiple repository or raw SQL operations under a single transactional scope.
 *
 * ---
 * ## Overview
 * This pattern (often known from DDD — Domain-Driven Design) ensures that:
 * - Multiple changes (possibly across several repositories) are committed atomically.
 * - If any operation fails, the entire unit rolls back automatically.
 * - Resource lifetime and rollback safety are managed via RAII.
 *
 * Internally, `UnitOfWork` wraps a `Transaction` (and thus a single pooled connection).
 *
 * ---
 * ## Example
 * ```cpp
 * using namespace Vix::orm;
 *
 * ConnectionPool pool("localhost", "root", "pwd", "shop");
 *
 * try {
 *     UnitOfWork uow(pool);
 *     auto& conn = uow.conn(); // Connection&
 *
 *     auto st = conn.prepare("INSERT INTO users(name,age) VALUES(?,?)");
 *     st->bind(1, "Alice");
 *     st->bind(2, 25);
 *     st->exec();
 *
 *     uow.commit(); // ✅ persist all at once
 * } catch (const DBError& e) {
 *     // automatic rollback when leaving scope if commit() wasn't called
 * }
 * ```
 *
 * ---
 * ## Design notes
 * - `UnitOfWork` should be **short-lived**, typically scoped to one logical
 *   business operation or HTTP request.
 * - It holds exactly one `Transaction` instance and one DB connection.
 * - Thread confinement: do not share the same instance across threads.
 * - Nesting multiple `UnitOfWork` objects on the same thread is discouraged.
 *
 * ---
 * ## See also
 * - `Transaction` — the underlying RAII transaction manager.
 * - `ConnectionPool` — provides pooled, thread-safe DB connections.
 */

namespace Vix::orm
{
    /**
     * @class UnitOfWork
     * @brief Encapsulates a transaction-scoped DB connection for grouped operations.
     *
     * The Unit of Work pattern coordinates changes to multiple objects
     * that must be persisted atomically.
     */
    class UnitOfWork
    {
        Transaction tx_; ///< Underlying RAII transaction manager.

    public:
        /**
         * @brief Begins a new unit of work (starts a DB transaction).
         * @param pool Reference to a shared ConnectionPool.
         */
        explicit UnitOfWork(ConnectionPool &pool) : tx_(pool) {}

        /**
         * @brief Commits the current unit of work.
         *
         * All operations performed using this instance’s connection
         * are persisted to the database.
         */
        void commit() { tx_.commit(); }

        /**
         * @brief Rolls back the unit of work.
         *
         * Cancels all uncommitted changes made within this transaction.
         * Safe to call multiple times.
         */
        void rollback() { tx_.rollback(); }

        /**
         * @brief Access the underlying driver-agnostic connection.
         * @return A reference to the active `Connection`.
         *
         * Use this to pass the connection to repositories or execute
         * manual SQL operations within the same transaction.
         */
        Connection &conn() { return tx_.conn(); }
    };
} // namespace Vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
