#ifndef VIX_MIGRATION_HPP
#define VIX_MIGRATION_HPP

#include <string>
#include <vix/orm/Drivers.hpp>

/**
 * @file Migration.hpp
 * @brief Base interface for database schema migrations in Vix ORM.
 *
 * A **Migration** represents an atomic schema evolution step with a stable identifier.
 * Each migration must implement:
 *  - `id()`   : a unique, deterministic ID (e.g., timestamp-based).
 *  - `up()`   : the forward change (apply/upgrade).
 *  - `down()` : the reverse change (revert/downgrade), when possible.
 *
 * Migrations are executed by `MigrationsRunner` in a defined order (typically
 * lexicographical by `id()`), and *should* be wrapped in transactions when the
 * underlying database supports them.
 *
 * ---
 * ## ID guidelines
 * - Prefer **timestamp-based IDs** for ordering: `YYYY_MM_DD_HHMMSS_<slug>`
 *   e.g. `"2025_10_10_121530_create_users"`.
 * - Keep them **unique** and **immutable** once published.
 *
 * ---
 * ## Transaction best practices
 * - When possible, each `up()` / `down()` should be **transactional**:
 *   - Start a transaction in the runner (or inside up/down), execute all statements,
 *     then `commit()` or `rollback()` on error.
 * - Some DDL statements may be auto-committing on certain RDBMS — document this in
 *   your migration if necessary.
 *
 * ---
 * ## Irreversible migrations
 * - If `down()` cannot be implemented safely (e.g., destructive data changes),
 *   it may throw or be intentionally empty but **must document** the behavior.
 *
 * ---
 * ## Example
 * ```cpp
 * #include <vix/orm/Migration.hpp>
 *
 * class CreateUsers final : public Vix::orm::Migration {
 * public:
 *   std::string id() const override { return "2025_10_10_120000_create_users"; }
 *
 *   void up(Vix::orm::Connection& c) override {
 *     auto st = c.prepare(
 *       "CREATE TABLE IF NOT EXISTS users ("
 *       "  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
 *       "  name  VARCHAR(120) NOT NULL,"
 *       "  email VARCHAR(190) NOT NULL,"
 *       "  age   INT NOT NULL,"
 *       "  PRIMARY KEY (id)"
 *       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;"
 *     );
 *     st->exec();
 *   }
 *
 *   void down(Vix::orm::Connection& c) override {
 *     auto st = c.prepare("DROP TABLE IF EXISTS users");
 *     st->exec();
 *   }
 * };
 * ```
 *
 * ---
 * ## Notes
 * - Use `Connection::prepare()` and `Statement::exec()` for DDL/DML.
 * - Keep migrations **idempotent** when feasible (e.g., `IF NOT EXISTS`).
 * - For data migrations (backfilling columns), split schema vs data steps for clarity.
 *
 * @see MigrationsRunner.hpp
 * @see Drivers.hpp
 */

namespace Vix::orm
{
    /**
     * @brief Abstract base class for schema migrations.
     *
     * Implement this interface to represent a single schema change. The
     * `MigrationsRunner` will orchestrate calling `up()`/`down()` in order.
     */
    struct Migration
    {
        virtual ~Migration() = default;

        /**
         * @brief Stable unique identifier for the migration.
         * @return A string like `"2025_10_10_120000_create_users"`.
         *
         * @note IDs are used to order and track applied migrations.
         *       Once published, do not change them.
         */
        virtual std::string id() const = 0;

        /**
         * @brief Apply the migration (upgrade the schema).
         *
         * Use the provided `Connection` to execute DDL/DML statements.
         * The caller (runner) may wrap this in a transaction if supported.
         *
         * @throws DBError (or derived) on failure.
         */
        virtual void up(Connection &c) = 0;

        /**
         * @brief Revert the migration (downgrade the schema).
         *
         * Provide an inverse of `up()` when feasible. If not possible,
         * the implementation may throw to indicate irreversibility.
         *
         * @throws DBError (or derived) on failure or if unsupported.
         */
        virtual void down(Connection &c) = 0;
    };
} // namespace Vix::orm

#endif // VIX_MIGRATION_HPP
