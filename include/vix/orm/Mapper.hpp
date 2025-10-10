#ifndef VIX_MAPPER_HPP
#define VIX_MAPPER_HPP

#include <vix/orm/Drivers.hpp>
#include <string>
#include <vector>
#include <any>

/**
 * @file Mapper.hpp
 * @brief Template contract for object-relational mapping in Vix ORM.
 *
 * The `Mapper<T>` template defines how a C++ domain object `T`
 * maps to and from a database record.
 *
 * Users must **specialize** `Mapper<T>` for each of their entity types
 * (e.g., `User`, `Product`, `Order`), providing conversion logic:
 *
 *  - `fromRow(const ResultRow&)` → create a `T` from a DB row.
 *  - `toInsertParams(const T&)`  → list of (column, value) pairs for INSERT.
 *  - `toUpdateParams(const T&)`  → list of (column, value) pairs for UPDATE.
 *
 * ---
 * ## Design goals
 * - **Separation of concerns:** Entities stay pure and unaware of SQL.
 * - **Static mapping:** All mappings are compile-time specializations; no RTTI or reflection.
 * - **Driver independence:** Works with any backend (MySQL, SQLite, PostgreSQL…).
 *
 * ---
 * ## Example
 * ```cpp
 * struct User {
 *     std::int64_t id{};
 *     std::string name;
 *     std::string email;
 *     int age{};
 * };
 *
 * namespace Vix::orm {
 * template <>
 * struct Mapper<User> {
 *     static User fromRow(const ResultRow &r) {
 *         User u;
 *         u.id    = r.getInt64(0);
 *         u.name  = r.getString(1);
 *         u.email = r.getString(2);
 *         u.age   = static_cast<int>(r.getInt64(3));
 *         return u;
 *     }
 *
 *     static std::vector<std::pair<std::string, std::any>> toInsertParams(const User &u) {
 *         return {
 *             {"name",  u.name},
 *             {"email", u.email},
 *             {"age",   u.age}
 *         };
 *     }
 *
 *     static std::vector<std::pair<std::string, std::any>> toUpdateParams(const User &u) {
 *         return {
 *             {"name",  u.name},
 *             {"email", u.email},
 *             {"age",   u.age}
 *         };
 *     }
 * };
 * } // namespace Vix::orm
 * ```
 *
 * ---
 * ## Notes
 * - The **order of columns** in `fromRow` should match the SQL `SELECT` statement.
 * - `std::any` is used for portability; the driver layer casts to driver-specific types.
 * - Always use **column names** in `toInsertParams` / `toUpdateParams`, not indices.
 * - `toInsertParams` should **exclude auto-increment primary keys**.
 *
 * ---
 * ## Common mistakes
 * ❌ Forgetting to skip `id` in insert params
 * ❌ Returning params in a different order from SQL columns
 * ❌ Using incorrect type (e.g., `std::string` instead of `int`)
 *
 * ---
 * ## Future roadmap
 * - Support for custom converters (e.g., `enum <-> int`, `date <-> string`).
 * - Integration with Vix validation utilities.
 * - Optional compile-time reflection for trivial structs.
 *
 * @see Repository.hpp
 * @see Drivers.hpp
 */

namespace Vix::orm
{
    /**
     * @brief Base template defining the mapping contract between C++ types and SQL.
     *
     * @tparam T The domain type to map.
     *
     * Specialize this struct for each entity you want to persist.
     * The default template is undefined and will produce a linker error if used directly.
     */
    template <class T>
    struct Mapper
    {
        /**
         * @brief Create an instance of `T` from the current `ResultRow`.
         *
         * @param row The current row in a `ResultSet`.
         * @return A fully constructed instance of `T`.
         */
        static T fromRow(const ResultRow &row);

        /**
         * @brief Return parameters for an `INSERT` statement.
         *
         * @param v Entity instance.
         * @return Vector of (column, value) pairs for SQL placeholders.
         */
        static std::vector<std::pair<std::string, std::any>> toInsertParams(const T &v);

        /**
         * @brief Return parameters for an `UPDATE` statement.
         *
         * @param v Entity instance.
         * @return Vector of (column, value) pairs for SQL placeholders.
         */
        static std::vector<std::pair<std::string, std::any>> toUpdateParams(const T &v);
    };
} // namespace Vix::orm

#endif // VIX_MAPPER_HPP
