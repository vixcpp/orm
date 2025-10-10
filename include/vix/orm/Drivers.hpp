#ifndef VIX_DRIVERS_HPP
#define VIX_DRIVERS_HPP

#include <any>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

/**
 * @file Drivers.hpp
 * @brief Core database driver interfaces for Vix ORM.
 *
 * This header defines the **driver-agnostic contracts** used by Vix ORM:
 * - `Connection`: a live connection to a database (begin/commit/rollback).
 * - `Statement`: a prepared statement with positional parameter binding.
 * - `ResultSet` / `ResultRow`: forward-only result navigation and row access.
 *
 * These abstractions allow plugging different backends (MySQL, SQLite, PostgreSQL, …)
 * without changing higher-level code (Repositories, Unit of Work, etc.).
 *
 * ---
 * ## Indexing rules
 * - **Parameters** (`Statement::bind`): **1-based** (matches most SQL client APIs).
 * - **Columns** (`ResultRow::get*`, `ResultRow::isNull`): **0-based**.
 *
 * ---
 * ## Lifetimes & ownership
 * - A `Statement` is owned by the caller (usually created via `Connection::prepare`).
 * - A `ResultSet` is **forward-only**; calling `next()` advances the cursor.
 * - A `ResultRow` returned by `ResultSet::row()` refers to the **current row** and
 *   is only valid **until the next `next()` call** or destruction of the `ResultSet`.
 * - All interfaces use `virtual` destructors to allow safe deletion via base pointers.
 *
 * ---
 * ## Error model
 * Driver implementations should report failures via exceptions (e.g., `std::runtime_error`
 * or the framework-specific `Vix::orm::DBError`), making errors explicit and catchable
 * at repository/service level.
 *
 * ---
 * ## Thread-safety
 * - **Connections** are generally **not** thread-safe. Acquire one per thread/task,
 *   e.g. through `ConnectionPool` and `PooledConn`.
 * - `Statement` and `ResultSet` are **not** thread-safe; do not share across threads.
 *
 * ---
 * ## Example usage
 * @code
 * using namespace Vix::orm;
 *
 * // Acquire a connection (via pool or direct driver factory).
 * std::unique_ptr<Statement> st = conn.prepare("INSERT INTO users(name, age) VALUES(?, ?)");
 * st->bind(1, std::string{"Alice"}); // 1-based
 * st->bind(2, 28);
 * auto affected = st->exec();
 *
 * // Query example
 * auto q = conn.prepare("SELECT id, name, age FROM users WHERE age >= ?");
 * q->bind(1, 18);
 * auto rs = q->query();
 * while (rs->next()) {
 *   auto r = rs->row();               // snapshot/view of the current row
 *   auto id   = r->getInt64(0);       // 0-based column
 *   auto name = r->getString(1);
 *   auto age  = r->getInt64(2);
 * }
 * @endcode
 */

namespace Vix::orm
{
    /**
     * @brief Read-only view of the current row within a `ResultSet`.
     *
     * Implementations should provide **0-based** column accessors and perform
     * type conversions consistent with the underlying driver (throw on mismatch).
     *
     * The object returned by `ResultSet::row()` is valid **until**:
     * - the next call to `ResultSet::next()`, or
     * - the `ResultSet` is destroyed.
     */
    struct ResultRow
    {
        virtual ~ResultRow() = default;

        /**
         * @brief Whether the given column is SQL NULL.
         * @param i 0-based column index.
         */
        virtual bool isNull(std::size_t i) const = 0;

        /**
         * @brief Read a text value from column `i`.
         * @param i 0-based column index.
         * @throws on NULL or incompatible type.
         */
        virtual std::string getString(std::size_t i) const = 0;

        /**
         * @brief Read a 64-bit integer from column `i`.
         * @param i 0-based column index.
         * @throws on NULL or incompatible type.
         */
        virtual std::int64_t getInt64(std::size_t i) const = 0;

        /**
         * @brief Read a floating-point value from column `i`.
         * @param i 0-based column index.
         * @throws on NULL or incompatible type.
         */
        virtual double getDouble(std::size_t i) const = 0;
    };

    /**
     * @brief Forward-only result set, navigated with `next()`.
     *
     * Typical usage:
     * @code
     * auto rs = stmt->query();
     * while (rs->next()) {
     *   auto r = rs->row();
     *   // read columns...
     * }
     * @endcode
     */
    struct ResultSet
    {
        virtual ~ResultSet() = default;

        /**
         * @brief Advance to the next row.
         * @return true if a row is available, false if end-of-stream.
         */
        virtual bool next() = 0;

        /**
         * @brief Number of columns in the result set.
         */
        virtual std::size_t cols() const = 0;

        /**
         * @brief Access the current row (valid until next `next()`).
         * @return A lightweight object representing the current row.
         */
        virtual std::unique_ptr<ResultRow> row() const = 0;
    };

    /**
     * @brief Prepared SQL statement with positional parameter binding.
     *
     * Parameters are **1-based** to match common SQL client APIs.
     * Call `exec()` for statements that modify data, and `query()` for SELECTs.
     */
    struct Statement
    {
        virtual ~Statement() = default;

        /**
         * @brief Bind a value to a positional parameter.
         * @param idx 1-based parameter index.
         * @param v   Value to bind: supported types are driver-specific but
         *            typically include `int`, `std::int64_t`, `unsigned`,
         *            `double`, `float`, `bool`, `const char*`, `std::string`.
         * @throws on unsupported types or driver errors.
         */
        virtual void bind(std::size_t idx, const std::any &v) = 0;

        /**
         * @brief Execute a SELECT statement.
         * @return A forward-only result set.
         * @throws on statements that do not produce results or on driver errors.
         */
        virtual std::unique_ptr<ResultSet> query() = 0;

        /**
         * @brief Execute an INSERT/UPDATE/DELETE (or DDL).
         * @return Number of affected rows (driver semantics).
         * @throws on driver errors.
         */
        virtual std::uint64_t exec() = 0;
    };

    /**
     * @brief Live connection to a database.
     *
     * A `Connection` creates prepared statements, manages transactions,
     * and provides access to database-specific functions like `lastInsertId()`.
     *
     * @warning Connections are usually **not** thread-safe. Use a `ConnectionPool`
     * and acquire one per concurrent task (see `PooledConn`).
     */
    struct Connection
    {
        virtual ~Connection() = default;

        /**
         * @brief Prepare a SQL statement.
         */
        virtual std::unique_ptr<Statement> prepare(std::string_view sql) = 0;

        /**
         * @brief Begin a transaction (turn off autocommit if applicable).
         */
        virtual void begin() = 0;

        /**
         * @brief Commit current transaction and restore autocommit (if any).
         */
        virtual void commit() = 0;

        /**
         * @brief Rollback current transaction and restore autocommit (if any).
         */
        virtual void rollback() = 0;

        /**
         * @brief Return the last auto-generated ID in the current session.
         * @return The driver-specific 64-bit identifier.
         * @throws if not supported or no ID available in the current context.
         */
        virtual std::uint64_t lastInsertId() = 0;
    };

} // namespace Vix::orm

#endif // VIX_DRIVERS_HPP
