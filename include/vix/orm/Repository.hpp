#ifndef VIX_REPOSITORY_HPP
#define VIX_REPOSITORY_HPP

#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/Mapper.hpp>
#include <optional>
#include <vector>
#include <string>

/**
 * @file Repository.hpp
 * @brief High-level base repository abstraction for CRUD operations.
 *
 * The `BaseRepository<T>` class provides a ready-to-use, type-safe interface
 * for performing **CRUD operations** (Create, Read, Update, Delete)
 * using a shared connection pool.
 *
 * ---
 * ## Overview
 * Each entity `T` is expected to define a static `Mapper<T>` specialization
 * (see `Mapper.hpp`) that knows how to:
 * - Serialize `T` → (column, value) pairs for insertion and update.
 * - Deserialize a database row → `T`.
 *
 * The repository automatically builds SQL for standard patterns:
 * ```sql
 * INSERT INTO table (col1,col2,...) VALUES (?, ?, ...)
 * SELECT * FROM table WHERE id = ?
 * UPDATE table SET col1=?, col2=?, ... WHERE id=?
 * DELETE FROM table WHERE id=?
 * ```
 *
 * ---
 * ## Example usage
 * ```cpp
 * struct User {
 *     int id;
 *     std::string name;
 *     int age;
 * };
 *
 * namespace Vix::orm {
 *     template <>
 *     struct Mapper<User> {
 *         static User fromRow(const ResultRow& r) {
 *             return {r.getInt64(0), r.getString(1), static_cast<int>(r.getInt64(2))};
 *         }
 *         static auto toInsertParams(const User& u) {
 *             return std::vector{
 *                 std::make_pair("name", std::any{u.name}),
 *                 std::make_pair("age", std::any{u.age})
 *             };
 *         }
 *         static auto toUpdateParams(const User& u) {
 *             return toInsertParams(u);
 *         }
 *     };
 * }
 *
 * // Usage
 * ConnectionPool pool("localhost", "root", "pwd", "mydb");
 * BaseRepository<User> repo(pool, "users");
 *
 * User u{0, "Alice", 25};
 * auto id = repo.create(u);
 * auto user = repo.findById(id);
 * repo.updateById(id, User{id, "Alice Updated", 26});
 * repo.removeById(id);
 * ```
 *
 * ---
 * ## Thread-safety
 * All database access is synchronized through the internal `ConnectionPool`.
 * Each call to `create`, `updateById`, `removeById` obtains and releases
 * a connection automatically using RAII (`PooledConn`).
 *
 * ---
 * ## Notes
 * - `findById()` returns `std::nullopt` for now; once `ResultSet` is complete,
 *   it will return a populated `T`.
 * - Errors throw `DBError` or derived exceptions (e.g., `NotFound`).
 *
 * @tparam T The entity type with a registered `Mapper<T>`.
 */

namespace Vix::orm
{

    /**
     * @class BaseRepository
     * @tparam T Entity type (must have a `Mapper<T>` specialization).
     *
     * Provides the four basic CRUD methods:
     *  - `create()`     → INSERT
     *  - `findById()`   → SELECT
     *  - `updateById()` → UPDATE
     *  - `removeById()` → DELETE
     *
     * Each method uses `ConnectionPool` and `Mapper<T>` to generate
     * parameterized SQL statements.
     */
    template <class T>
    class BaseRepository
    {
        ConnectionPool &pool_; ///< Shared connection pool
        std::string table_;    ///< Target table name

    public:
        /**
         * @brief Construct a repository bound to a specific table.
         * @param pool Connection pool to draw from.
         * @param table Table name for this repository.
         */
        BaseRepository(ConnectionPool &pool, std::string table)
            : pool_(pool), table_(std::move(table)) {}

        /**
         * @brief Create a new record from entity `v`.
         * @return The auto-increment ID (if supported by the driver).
         */
        std::uint64_t create(const T &v)
        {
            auto params = Mapper<T>::toInsertParams(v);

            std::string cols, qs;
            cols.reserve(64);
            qs.reserve(32);

            for (std::size_t i = 0; i < params.size(); ++i)
            {
                cols += params[i].first;
                qs += "?";
                if (i + 1 < params.size())
                {
                    cols += ",";
                    qs += ",";
                }
            }

            auto sql = "INSERT INTO " + table_ + " (" + cols + ") VALUES (" + qs + ")";
            PooledConn pc(pool_);
            auto st = pc.get().prepare(sql);

            for (std::size_t i = 0; i < params.size(); ++i)
                st->bind(i + 1, params[i].second);

            st->exec();
            return pc.get().lastInsertId();
        }

        /**
         * @brief Retrieve one entity by ID (if exists).
         * @return `std::optional<T>` — `std::nullopt` if not found.
         *
         * @note Currently returns `std::nullopt` until `ResultSet`
         *       implementation is complete.
         */
        std::optional<T> findById(std::int64_t id)
        {
            auto sql = "SELECT * FROM " + table_ + " WHERE id = ? LIMIT 1";
            PooledConn pc(pool_);
            auto st = pc.get().prepare(sql);
            st->bind(1, id);
            auto rs = st->query();

            // TODO: Once ResultSet wrapper is ready:
            // if (rs->next()) return Mapper<T>::fromRow(*rs->row());

            return std::nullopt;
        }

        /**
         * @brief Update record by its primary key.
         * @param id The record ID to update.
         * @param v  The entity values (columns to update).
         * @return The number of affected rows.
         */
        std::uint64_t updateById(std::int64_t id, const T &v)
        {
            auto params = Mapper<T>::toUpdateParams(v);

            std::string set;
            set.reserve(128);
            for (std::size_t i = 0; i < params.size(); ++i)
            {
                set += params[i].first + "=?";
                if (i + 1 < params.size())
                    set += ",";
            }

            auto sql = "UPDATE " + table_ + " SET " + set + " WHERE id=?";
            PooledConn pc(pool_);
            auto st = pc.get().prepare(sql);

            std::size_t idx = 1;
            for (auto &[k, vv] : params)
                st->bind(idx++, vv);
            st->bind(idx, id);

            return st->exec();
        }

        /**
         * @brief Delete a record by ID.
         * @param id The record ID.
         * @return Number of rows removed (0 or 1).
         */
        std::uint64_t removeById(std::int64_t id)
        {
            PooledConn pc(pool_);
            auto st = pc.get().prepare("DELETE FROM " + table_ + " WHERE id = ?");
            st->bind(1, id);
            return st->exec();
        }
    };

} // namespace Vix::orm

#endif // VIX_REPOSITORY_HPP
