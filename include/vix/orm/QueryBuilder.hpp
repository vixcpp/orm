#ifndef VIX_QUERY_BUILDER
#define VIX_QUERY_BUILDER

#include <string>
#include <string_view>
#include <vector>
#include <any>

/**
 * @file QueryBuilder.hpp
 * @brief Minimal fluent builder for SQL strings and positional parameters.
 *
 * `QueryBuilder` helps you compose SQL fragments while collecting a parallel
 * list of **positional parameters**. It does **not** execute queries; you pass
 * `sql()` and `params()` to a `Statement` obtained via `Connection::prepare()`.
 *
 * ---
 * ## Key points
 * - The SQL you build should use `?` placeholders for parameters.
 * - Parameters are stored in insertion order; you must bind them as **1-based**
 *   indices on the `Statement` (`bind(1, params[0])`, etc.).
 * - `raw()` appends literal SQL, `space()` just inserts a space, `param()` adds
 *   a value to the internal parameter list (you still need to put `?` in SQL).
 *
 * ---
 * ## Example
 * ```cpp
 * using namespace Vix::orm;
 *
 * QueryBuilder qb;
 * qb.raw("SELECT id,name,age FROM users WHERE age >= ?")
 *   .param(18)
 *   .raw(" AND name LIKE ?")
 *   .param(std::string{"A%"});
 *
 * auto st = conn.prepare(qb.sql());
 * const auto& ps = qb.params();
 * for (std::size_t i = 0; i < ps.size(); ++i)
 *     st->bind(i + 1, ps[i]); // 1-based indices
 *
 * auto rs = st->query();
 * ```
 *
 * ---
 * ## Safety notes
 * - Prefer `param()` for values instead of concatenating literals into SQL.
 * - Only use `raw()` for keywords, identifiers already validated, or trusted
 *   fragments built by your application (e.g., whitelists).
 *
 * @see Drivers.hpp (Statement::bind, Connection::prepare)
 */

namespace Vix::orm
{
    /**
     * @brief Fluent builder for SQL strings and ordered parameters.
     *
     * The builder is intentionally minimal and header-only. It does not try
     * to escape identifiers or values; parameterization is the caller’s job
     * using `?` placeholders and `Statement::bind`.
     */
    class QueryBuilder
    {
        std::string sql_;              ///< Accumulated SQL (with `?` placeholders).
        std::vector<std::any> params_; ///< Positional parameters in insertion order.

    public:
        /**
         * @brief Append raw SQL to the buffer (no escaping).
         * @param s SQL fragment (e.g., `"SELECT * FROM users WHERE id=?"`).
         * @return *this for chaining.
         *
         * @warning Do not inject user-provided values here. Use `param()` and `?`.
         */
        QueryBuilder &raw(std::string_view s)
        {
            sql_.append(s);
            return *this;
        }

        /**
         * @brief Append a single space (useful in fluent chains).
         * @return *this for chaining.
         */
        QueryBuilder &space()
        {
            sql_.push_back(' ');
            return *this;
        }

        /**
         * @brief Add a positional parameter value (to bind later).
         * @param v Any supported driver type (e.g., `int`, `int64_t`, `double`,
         *          `float`, `bool`, `const char*`, `std::string`).
         * @return *this for chaining.
         *
         * @note Remember to place a `?` in the SQL where this value belongs.
         */
        QueryBuilder &param(std::any v)
        {
            params_.push_back(std::move(v));
            return *this;
        }

        /**
         * @brief Final SQL string (with `?` placeholders).
         */
        const std::string &sql() const { return sql_; }

        /**
         * @brief Collected parameters corresponding to the `?` placeholders.
         *
         * Bind them as 1-based indices on your prepared `Statement`.
         */
        const std::vector<std::any> &params() const { return params_; }
    };
} // namespace Vix::orm

#endif // VIX_QUERY_BUILDER
