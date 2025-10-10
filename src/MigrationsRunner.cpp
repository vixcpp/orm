/**
 * @file MigrationsRunner.cpp
 * @brief Contributor documentation — migration execution engine internals.
 *
 * This translation unit implements the core logic for executing all registered
 * migrations in the order they were added to the runner.
 *
 *  ──────────────────────────────────────────────────────────────────────────
 *  Contents
 *  ──────────────────────────────────────────────────────────────────────────
 *   1) Execution model overview
 *   2) Error propagation policy
 *   3) Transaction handling (future)
 *   4) Tracking and idempotency roadmap
 *   5) Logging and CLI integration notes
 *
 *  -------------------------------------------------------------------------
 *  1) Execution model overview
 *  -------------------------------------------------------------------------
 *  The runner simply iterates through the vector of registered `Migration*`
 *  and calls `up(Connection&)` on each.
 *
 *  The current design is intentionally **minimalistic and deterministic**:
 *   - No implicit sorting (executions follow registration order).
 *   - No ownership of migration objects.
 *   - No transaction wrapping (delegated to `Migration::up()` or higher-level runner).
 *   - No persistence of state (tracking table planned for later).
 *
 *  In future releases, this runner may support:
 *   - Dependency graphs (A depends on B)
 *   - Parallel migration batches
 *   - Auto-rollback on partial failures
 *   - Persistent tracking of applied migrations (`vix_migrations` table)
 *
 *  -------------------------------------------------------------------------
 *  2) Error propagation policy
 *  -------------------------------------------------------------------------
 *  - Errors thrown by any migration propagate as-is (`DBError` or derived).
 *  - The loop **stops immediately** upon failure.
 *  - No rollback is attempted automatically — the calling context (e.g., CLI)
 *    is responsible for recovery (logging, rollback, exit code).
 *
 *  -------------------------------------------------------------------------
 *  3) Transaction handling (future)
 *  -------------------------------------------------------------------------
 *  We deliberately do not wrap all migrations in a global transaction, because:
 *   - DDL statements (CREATE TABLE, ALTER TABLE) often auto-commit.
 *   - Some backends (SQLite, MySQL) cannot roll back such DDL reliably.
 *   - Fine-grained transactions are better handled inside each `Migration::up()`.
 *
 *  A future enhancement could detect transactional capabilities via
 *  driver introspection and optionally group compatible migrations.
 *
 *  -------------------------------------------------------------------------
 *  4) Tracking and idempotency roadmap
 *  -------------------------------------------------------------------------
 *  A planned addition is a tracking table (`vix_migrations`) with:
 *   - `id` (varchar, primary key)
 *   - `applied_at` (timestamp)
 *   - `checksum` (optional hash for drift detection)
 *
 *  The runner would then:
 *   - Check applied migrations before execution
 *   - Skip those already marked as applied
 *   - Support `rollback <id>` using `Migration::down()`
 *
 *  -------------------------------------------------------------------------
 *  5) Logging and CLI integration notes
 *  -------------------------------------------------------------------------
 *  - Logging will be integrated via `spdlog` or the Vix core logger.
 *  - CLI commands like `vix orm migrate` and `vix orm rollback` will invoke
 *    this runner from the `vix_cli` module.
 *  - A verbose mode could report execution times, applied IDs, and SQL timing.
 *
 *  -------------------------------------------------------------------------
 *  Example flow (pseudo-code)
 *  -------------------------------------------------------------------------
 *  @code
 *  MigrationsRunner runner(conn);
 *  runner.add(&CreateUsers);
 *  runner.add(&AddOrders);
 *
 *  try {
 *      runner.runAll();
 *      spdlog::info("All migrations applied.");
 *  } catch (const DBError& e) {
 *      spdlog::error("Migration failed: {}", e.what());
 *      // rollback or halt deployment
 *  }
 *  @endcode
 */

#include <vix/orm/MigrationsRunner.hpp>

namespace Vix::orm
{
    /**
     * @brief Executes all registered migrations sequentially.
     *
     * Each migration’s `up()` method is invoked using the shared `Connection`.
     * The order of execution is the same as registration order.
     *
     * @throws DBError if any migration fails; stops immediately.
     */
    void MigrationsRunner::runAll()
    {
        for (auto *m : migs_)
        {
            // Future enhancement: add logging / transaction per migration
            // Example: spdlog::info("Applying migration {}", m->id());
            m->up(conn_);
        }
    }
} // namespace Vix::orm
