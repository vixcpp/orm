#ifndef VIX_MIGRATIONS_RUNNER_HPP
#define VIX_MIGRATIONS_RUNNER_HPP

#include <vix/orm/Migration.hpp>
#include <vector>

/**
 * @file MigrationsRunner.hpp
 * @brief Orchestrates the execution of database migrations in Vix ORM.
 *
 * The **MigrationsRunner** is responsible for applying schema changes
 * (instances of `Migration`) in a controlled and ordered manner.
 *
 * It takes a `Connection` reference and a list of migration objects, then:
 *  1. Runs each migration’s `up()` method sequentially via `runAll()`.
 *  2. Optionally, records applied migrations in a tracking table (future feature).
 *
 * ---
 * ## Responsibilities
 * - Apply schema changes in deterministic order (typically sorted by ID).
 * - Propagate errors as exceptions (no silent fail).
 * - Keep migrations idempotent (e.g., use `IF NOT EXISTS` in SQL).
 * - Allow easy integration with deployment tools or CLI commands (`vix orm migrate`).
 *
 * ---
 * ## Typical usage
 * ```cpp
 * #include <vix/orm/MigrationsRunner.hpp>
 *
 * int main() {
 *   // Acquire a database connection
 *   auto conn = make_mysql_conn("localhost", "root", "pass", "softadastra_db");
 *
 *   // Register migrations
 *   CreateUsers  createUsers;
 *   AddProducts  addProducts;
 *
 *   Vix::orm::MigrationsRunner runner{*conn};
 *   runner.add(&createUsers);
 *   runner.add(&addProducts);
 *
 *   // Execute all migrations
 *   try {
 *       runner.runAll();
 *       std::cout << "✅ Migrations applied successfully!" << std::endl;
 *   } catch (const Vix::orm::DBError& e) {
 *       std::cerr << "❌ Migration failed: " << e.what() << std::endl;
 *       return 1;
 *   }
 * }
 * ```
 *
 * ---
 * ## Notes
 * - Migrations are applied in **the order they were added** to the runner.
 *   (Future versions may support sorting or dependency graphs.)
 * - `Connection` is abstract; the same runner works for MySQL, SQLite, etc.
 * - A tracking table (`vix_migrations`) can be implemented externally to record progress.
 * - Rollback support (via `Migration::down()`) can be added in CI/CD pipelines.
 *
 * ---
 * ## Roadmap
 * - Persistent tracking table for applied migrations.
 * - CLI integration (`vix migrate`, `vix rollback`).
 * - Dry-run / preview mode.
 * - Automatic sorting by timestamped IDs.
 *
 * @see Migration.hpp
 * @see Connection.hpp
 * @see DBError
 */

namespace Vix::orm
{
    /**
     * @brief Executes a sequence of schema migrations on a given connection.
     *
     * @details
     * The runner is initialized with a `Connection&` and can register any
     * number of `Migration*` instances using `add()`.
     * Once configured, calling `runAll()` will execute each migration’s `up()`.
     */
    class MigrationsRunner
    {
        Connection &conn_;              ///< Shared connection used for all migrations.
        std::vector<Migration *> migs_; ///< Ordered list of migrations to apply.

    public:
        /**
         * @brief Construct a runner bound to a specific database connection.
         * @param c Reference to a connected database instance.
         */
        explicit MigrationsRunner(Connection &c) : conn_(c) {}

        /**
         * @brief Register a migration to be executed.
         * @param m Pointer to a `Migration` instance.
         *
         * @warning The runner does **not** take ownership of the pointer;
         * ensure the migration object outlives the runner.
         */
        void add(Migration *m) { migs_.push_back(m); }

        /**
         * @brief Execute all registered migrations sequentially.
         *
         * @throws DBError (or derived) if a migration fails.
         * Execution stops on the first failure.
         */
        void runAll();
    };
} // namespace Vix::orm

#endif // VIX_MIGRATIONS_RUNNER_HPP
