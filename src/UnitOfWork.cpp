/**
 * @file UnitOfWork.cpp
 * @brief Contributor documentation — internals & guidelines for UnitOfWork.
 *
 * This TU intentionally defines **no linker-visible symbols**. It documents
 * the design, invariants, and recommended practices for the Unit of Work
 * pattern in Vix ORM.
 *
 * ───────────────────────────────────────────────────────────────────────────
 *  Contents
 * ───────────────────────────────────────────────────────────────────────────
 *  1) Purpose & scope
 *  2) Lifetime & invariants
 *  3) Relationship with Transaction and Repository
 *  4) Threading & pooling constraints
 *  5) Error handling policy
 *  6) Usage patterns & anti-patterns
 *  7) Roadmap (nested UoW, isolation levels, cross-DB orchestration)
 *
 * ----------------------------------------------------------------------------
 * 1) Purpose & scope
 * ----------------------------------------------------------------------------
 *  UnitOfWork (UoW) groups multiple persistence operations under a single,
 *  atomic transaction boundary. It owns a `Transaction`, hence a single
 *  connection, and exposes `commit()`/`rollback()` with RAII safety.
 *
 *  Goals:
 *    - Make transactional boundaries explicit and easy to reason about.
 *    - Ensure rollback on early exit or exceptions (RAII via Transaction).
 *    - Provide a single connection for all repositories participating
 *      in the unit of work.
 *
 * ----------------------------------------------------------------------------
 * 2) Lifetime & invariants
 * ----------------------------------------------------------------------------
 *  - Constructing a UoW starts a DB transaction (`BEGIN`).
 *  - `commit()` commits and disables destructor-rollback.
 *  - `rollback()` rolls back and disables destructor-rollback.
 *  - If neither is called, destructor triggers rollback (no-throw).
 *  - The same connection is used for the entire UoW (`uow.conn()`).
 *
 *  Invariants:
 *    * UoW is **single-owner, single-scope**; do not share across threads.
 *    * `Transaction`'s `active_` flag reflects whether rollback is pending.
 *
 * ----------------------------------------------------------------------------
 * 3) Relationship with Transaction and Repository
 * ----------------------------------------------------------------------------
 *  - UoW wraps `Transaction` (RAII guard) → one pooled connection.
 *  - Repositories may accept a `Connection&` or be constructed with a pool.
 *    For coherence inside a UoW, prefer repositories that accept a `Connection&`
 *    or expose methods that can operate on the provided connection.
 *  - When repositories internally acquire connections from the pool,
 *    they will not participate in the same transaction unless explicitly
 *    passed `uow.conn()`. Contributors should encourage a repo API that
 *    allows injecting `Connection&` for multi-step atomic workflows.
 *
 * ----------------------------------------------------------------------------
 * 4) Threading & pooling constraints
 * ----------------------------------------------------------------------------
 *  - UoW is not thread-safe. It should remain confined to one thread/task.
 *  - `Transaction` returns its connection to the `ConnectionPool` when the UoW
 *    object is destroyed (after commit/rollback).
 *  - Do not hold long-lived references to `uow.conn()` beyond the UoW lifetime.
 *
 * ----------------------------------------------------------------------------
 * 5) Error handling policy
 * ----------------------------------------------------------------------------
 *  - `commit()`/`rollback()` may throw `DBError` if the driver reports errors.
 *  - The UoW destructor must not throw; rollback exceptions are swallowed to
 *    preserve stack unwinding guarantees.
 *  - Call sites decide retry/compensation policies; UoW is deliberately minimal.
 *
 * ----------------------------------------------------------------------------
 * 6) Usage patterns & anti-patterns
 * ----------------------------------------------------------------------------
 *  ✓ Patterns:
 *    - Group related repository calls that must succeed together.
 *    - Pass `uow.conn()` into repositories or execute prepared statements directly.
 *    - Keep UoW scopes short (per request / per command).
 *
 *  ✗ Anti-patterns:
 *    - Nesting UoWs for sub-operations (consider savepoints instead).
 *    - Sharing a single UoW across multiple threads.
 *    - Long-running UoWs that block connection pool capacity.
 *
 * ----------------------------------------------------------------------------
 * 7) Roadmap
 * ----------------------------------------------------------------------------
 *  - Nested UoW via savepoints:
 *      Provide `SavepointGuard` utilities (`SAVEPOINT`, `ROLLBACK TO`, `RELEASE`).
 *  - Isolation levels:
 *      Optional API to set isolation (READ COMMITTED, REPEATABLE READ, SERIALIZABLE)
 *      per UoW, depending on driver capabilities.
 *  - Cross-DB orchestration:
 *      Two-phase commit (2PC) or best-effort saga-like compensations are out of scope
 *      for now; if needed, keep as a higher-level orchestration module.
 *  - Observability:
 *      Hook points for logging/metrics (begin/commit/rollback timestamps, duration).
 */

#include <vix/orm/UnitOfWork.hpp>

// No additional code here; this TU is documentation-only.
// If internal helpers (e.g., isolation-level setters, savepoint guards)
// are introduced later, they can live here behind a private header.
