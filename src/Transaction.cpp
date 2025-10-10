/**
 * @file Transaction.cpp
 * @brief Contributor documentation — RAII transaction helper internals.
 *
 * This TU intentionally defines **no linker-visible symbols**. It documents
 * the design and invariants of `Vix::orm::Transaction` for contributors.
 *
 * ───────────────────────────────────────────────────────────────────────────
 *  Contents
 * ───────────────────────────────────────────────────────────────────────────
 *  1) Purpose & scope
 *  2) RAII model & lifetime
 *  3) Error handling & exception safety
 *  4) Thread-safety & pooling
 *  5) Nested transactions & savepoints (roadmap)
 *  6) Timeouts & deadlock handling (roadmap)
 *  7) Multi-driver considerations
 *
 * ----------------------------------------------------------------------------
 * 1) Purpose & scope
 * ----------------------------------------------------------------------------
 *  `Transaction` provides a minimal, predictable RAII wrapper around DB
 *  transactions:
 *    - On construction: acquires a pooled connection and calls `begin()`.
 *    - On destruction: if not committed, calls `rollback()` (no-throw).
 *    - On `commit()`: calls `commit()`, marks `active_ = false`.
 *    - On `rollback()`: calls `rollback()`, marks `active_ = false`.
 *
 *  The helper is intentionally small. It does not attempt retries,
 *  deadlock detection, or nesting logic; those are left to higher layers.
 *
 * ----------------------------------------------------------------------------
 * 2) RAII model & lifetime
 * ----------------------------------------------------------------------------
 *  - The transaction holds a `PooledConn` for its entire lifetime.
 *  - The same connection is used for all statements via `tx.conn()`.
 *  - When the `Transaction` object leaves scope, either:
 *      * `commit()` has been called → do nothing further.
 *      * otherwise → attempt `rollback()` inside a try/catch (no-throw).
 *
 *  Invariants:
 *   - `active_ == true` means "rollback pending on dtor".
 *   - After `commit()` or `rollback()`, `active_ == false`.
 *
 * ----------------------------------------------------------------------------
 * 3) Error handling & exception safety
 * ----------------------------------------------------------------------------
 *  - `commit()` / `rollback()` may throw driver-level exceptions (`DBError`).
 *  - The destructor **must not throw**. Rollback failures in the destructor
 *    are swallowed to preserve stack unwinding guarantees.
 *  - Call sites should wrap units of work in try/catch and decide policy:
 *      * retries (idempotent ops),
 *      * compensations,
 *      * or surfacing as 5xx to higher layers (web, CLI).
 *
 * ----------------------------------------------------------------------------
 * 4) Thread-safety & pooling
 * ----------------------------------------------------------------------------
 *  - `Transaction` itself is not thread-safe; it is expected to be confined
 *    to a single thread/task.
 *  - The underlying `ConnectionPool` is thread-safe, and `PooledConn` will
 *    return the connection to the pool on destruction (post-rollback/commit).
 *  - Do not share `tx.conn()` across threads.
 *
 * ----------------------------------------------------------------------------
 * 5) Nested transactions & savepoints (roadmap)
 * ----------------------------------------------------------------------------
 *  The current design does not support nested transactions. If a caller
 *  needs to demarcate sub-scopes, contributors may consider adding:
 *
 *    - `SavepointGuard` with:
 *        * ctor → `SAVEPOINT ...`
 *        * `release()` → `RELEASE SAVEPOINT ...`
 *        * dtor → `ROLLBACK TO SAVEPOINT ...` (no-throw)
 *
 *  This requires dialect-aware support (MySQL: `SAVEPOINT`, `ROLLBACK TO`,
 *  `RELEASE SAVEPOINT`). Expose via optional APIs to avoid breaking users.
 *
 * ----------------------------------------------------------------------------
 * 6) Timeouts & deadlock handling (roadmap)
 * ----------------------------------------------------------------------------
 *  - Add optional per-transaction timeout (driver/session variable or query
 *    hints); on expiry, ensure rollback on the same connection.
 *  - Deadlock detection (SQLSTATEs like 1213 in MySQL) could trigger a retry
 *    policy opt-in at the service layer (bounded attempts, backoff jitter).
 *
 * ----------------------------------------------------------------------------
 * 7) Multi-driver considerations
 * ----------------------------------------------------------------------------
 *  - The helper currently assumes a MySQL-like semantics for `begin/commit/rollback`.
 *  - For other drivers (SQLite/PostgreSQL), the semantics are compatible,
 *    but auto-commit and DDL transactional behavior differ.
 *  - Keep the API generic; additional driver-specific knobs (isolation level,
 *    read-only tx, deferrable constraints) should be optional extensions.
 *
 *  Example usage (for docs):
 *  --------------------------------------------------------------------------
 *  @code
 *  using namespace Vix::orm;
 *  try {
 *      Transaction tx(pool);
 *      auto& c = tx.conn();
 *      auto st = c.prepare("UPDATE accounts SET balance=balance-? WHERE id=?");
 *      st->bind(1, 100);
 *      st->bind(2, 42);
 *      st->exec();
 *      tx.commit();
 *  } catch (const DBError& e) {
 *      // log & surface
 *  }
 *  @endcode
 */

// No includes beyond the header are necessary here; this TU documents internals.
// If contributor helpers are added later (e.g., isolation-level setters),
// they can live here guarded by an internal header (not part of public API).

#include <vix/orm/Transaction.hpp>
