#ifndef VIX_ORM_CONNECTION_POOL_HPP
#define VIX_ORM_CONNECTION_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vix/orm/Drivers.hpp> // Expose Connection / Statement / ...

/**
 * @file ConnectionPool.hpp
 * @brief Thread-safe connection pooling system for database access in Vix ORM.
 *
 * This header defines the `ConnectionPool` class, which manages a configurable
 * set of database connections to improve performance and scalability.
 * It also defines the `PooledConn` RAII helper for automatic connection lifecycle
 * management.
 *
 * ---
 * ## Overview
 * Creating and destroying database connections on each query is expensive.
 * The `ConnectionPool` keeps a pool of reusable connections that can be
 * acquired and released on demand.
 *
 * Each connection is automatically released back into the pool when a
 * `PooledConn` instance goes out of scope, ensuring safe reuse even under
 * multithreaded workloads.
 *
 * Typical usage:
 * @code
 * using namespace Vix::orm;
 *
 * ConnectionPool pool{"tcp://127.0.0.1:3306", "root", "", "vixdb"};
 *
 * // Acquire a connection automatically via RAII
 * {
 *     PooledConn conn(pool);
 *     auto stmt = conn.get().prepare("INSERT INTO users (name, age) VALUES (?, ?)");
 *     stmt->bind(1, "Alice");
 *     stmt->bind(2, 28);
 *     stmt->exec();
 * } // <- connection automatically returned to pool here
 * @endcode
 *
 * ---
 * ## Thread safety
 * The pool is fully thread-safe. Each call to `acquire()` and `release()`
 * uses a mutex + condition variable to ensure safe concurrent access.
 *
 * ---
 * ## Configuration
 * The pool can be configured with `PoolConfig`:
 * @code
 * PoolConfig cfg{.min = 2, .max = 10};
 * ConnectionPool pool("tcp://127.0.0.1:3306", "root", "", "vixdb", cfg);
 * @endcode
 *
 * - **min**: minimum number of connections to maintain (pre-warming optional)
 * - **max**: maximum number of total connections
 *
 * ---
 * ## Related classes
 * - `Connection`  — driver-agnostic database connection interface.
 * - `PooledConn`  — RAII wrapper for a connection borrowed from the pool.
 *
 * ---
 * @note
 * The pool currently instantiates MySQL connections internally. Future releases
 * of Vix ORM will include support for SQLite and PostgreSQL.
 *
 * @see Drivers.hpp
 * @see Transaction.hpp
 */

namespace Vix::orm
{
    /**
     * @struct PoolConfig
     * @brief Configuration parameters for `ConnectionPool`.
     *
     * Defines the minimum and maximum number of connections that
     * can be maintained in the pool.
     */
    struct PoolConfig
    {
        /// Minimum number of pre-warmed connections (default = 1)
        std::size_t min = 1;

        /// Maximum number of total connections allowed (default = 8)
        std::size_t max = 8;
    };

    /**
     * @class ConnectionPool
     * @brief Thread-safe pool for managing reusable database connections.
     *
     * Handles acquisition and release of `Connection` instances,
     * automatically balancing the number of open connections.
     *
     * @details
     * When `acquire()` is called:
     * - If there is an available connection in the idle queue, it is reused.
     * - If not and the total count is below `max`, a new connection is created.
     * - Otherwise, the caller waits until a connection is released.
     *
     * @warning Always return connections to the pool using `release()`
     * or by using the `PooledConn` RAII helper.
     */
    class ConnectionPool
    {
        std::string host_, user_, pass_, db_;
        PoolConfig cfg_{};
        std::mutex m_;
        std::condition_variable cv_;
        std::queue<std::shared_ptr<Connection>> idle_; // interface, not concrete
        std::size_t total_ = 0;

    public:
        /**
         * @brief Constructs a new `ConnectionPool` instance.
         *
         * @param host Database host URI (e.g. `tcp://127.0.0.1:3306`)
         * @param user Database username
         * @param pass Database password
         * @param db   Database name (schema)
         * @param cfg  Optional configuration struct (`PoolConfig`)
         */
        ConnectionPool(std::string host,
                       std::string user,
                       std::string pass,
                       std::string db,
                       PoolConfig cfg = {})
            : host_(std::move(host)),
              user_(std::move(user)),
              pass_(std::move(pass)),
              db_(std::move(db)),
              cfg_(cfg) {}

        /**
         * @brief Acquires a connection from the pool.
         *
         * @return A shared pointer to a valid `Connection`.
         *
         * @note
         * This method blocks if all connections are in use
         * and the pool is at its maximum size.
         */
        std::shared_ptr<Connection> acquire();

        /**
         * @brief Releases a connection back into the pool.
         *
         * @param c The connection to return. If `nullptr`, it is ignored.
         *
         * @note
         * Users normally do not need to call this manually.
         * Use `PooledConn` instead for automatic release.
         */
        void release(std::shared_ptr<Connection> c);
    };

    /**
     * @class PooledConn
     * @brief RAII helper that automatically acquires and releases a pooled connection.
     *
     * When a `PooledConn` object is created, it automatically acquires a
     * connection from the given pool. When it goes out of scope, the connection
     * is automatically released back to the pool.
     *
     * This pattern ensures safe connection reuse even when exceptions occur.
     *
     * Example:
     * @code
     * PooledConn conn(pool);
     * auto stmt = conn.get().prepare("DELETE FROM users WHERE id=?");
     * stmt->bind(1, 42);
     * stmt->exec();
     * @endcode
     */
    class PooledConn final
    {
        ConnectionPool &pool_;
        std::shared_ptr<Connection> c_; // interface

    public:
        /**
         * @brief Constructs a `PooledConn` and immediately acquires a connection.
         * @param p Reference to the connection pool to borrow from.
         */
        explicit PooledConn(ConnectionPool &p) : pool_(p), c_(p.acquire()) {}

        /**
         * @brief Automatically releases the connection when the object goes out of scope.
         */
        ~PooledConn()
        {
            if (c_)
                pool_.release(std::move(c_));
        }

        /**
         * @brief Returns a reference to the underlying `Connection`.
         */
        Connection &get() { return *c_; }
    };

} // namespace Vix::orm

#endif // VIX_ORM_CONNECTION_POOL_HPP
