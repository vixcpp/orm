#ifndef VIX_ERRORS_HPP
#define VIX_ERRORS_HPP

#include <string>
#include <stdexcept>

/**
 * @file Errors.hpp
 * @brief Exception hierarchy for Vix ORM.
 *
 * Vix ORM reports runtime failures via a small set of exception types
 * that inherit from `std::runtime_error`. This header declares:
 *
 * - `DBError`    : generic database/runtime error (base class).
 * - `TxnError`   : transaction-related error (begin/commit/rollback).
 * - `NotFound`   : entity or record not found (non-fatal control flow).
 *
 * ---
 * ## Usage principles
 * - Throw **`DBError`** for driver failures (connect, prepare, bind, exec, I/O).
 * - Throw **`TxnError`** for transactional boundary issues (begin/commit/rollback).
 * - Throw **`NotFound`** when an entity is expected but missing (e.g., `findById`).
 *
 * These types are intentionally minimal and header-only-friendly.
 * Drivers and repositories may provide richer messages by passing a string to the
 * constructor (e.g., SQL state, driver message, context).
 *
 * ---
 * ## Typical handling
 * ```cpp
 * try {
 *   auto id = users.create(u);
 *   auto item = users.findById(id);
 *   if (!item) throw Vix::orm::NotFound{"user not found after insert"};
 * } catch (const Vix::orm::NotFound& nf) {
 *   // handle absence (404); not necessarily an error in business logic
 * } catch (const Vix::orm::TxnError& tx) {
 *   // rollback already attempted at a higher layer; log & surface 500
 * } catch (const Vix::orm::DBError& db) {
 *   // generic DB problem: connection lost, bind error, syntax, etc.
 * } catch (const std::exception& e) {
 *   // fallback safety net
 * }
 * ```
 *
 * ---
 * ## Recommendations
 * - Include enough context in messages (table, SQL, parameters when safe).
 * - Avoid exposing secrets (never log raw passwords/connection strings).
 * - Let higher layers (services/controllers) map exceptions to HTTP status codes:
 *   - `NotFound` → 404
 *   - `TxnError` / `DBError` → 500 (or 409/422 when appropriate)
 *
 * @see Drivers.hpp
 * @see Repository.hpp
 * @see Transaction.hpp
 */

namespace Vix::orm
{
    /**
     * @brief Base class for all ORM runtime errors.
     *
     * Thrown for generic database/driver failures and unexpected states.
     * Construct with a descriptive message:
     * @code
     * throw Vix::orm::DBError{"prepare failed: " + driver_message};
     * @endcode
     */
    struct DBError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /**
     * @brief Transaction-related error (begin/commit/rollback).
     *
     * Use when transactional boundaries fail due to driver/state errors.
     * Example:
     * @code
     * try { conn.commit(); }
     * catch (const std::exception& e) { throw Vix::orm::TxnError{e.what()}; }
     * @endcode
     */
    struct TxnError : DBError
    {
        using DBError::DBError;
    };

    /**
     * @brief Indicates that the requested entity or row was not found.
     *
     * Prefer returning `std::optional<T>` from read APIs; throw `NotFound`
     * only when absence is considered exceptional in the calling context.
     */
    struct NotFound : DBError
    {
        using DBError::DBError;
    };
} // namespace Vix::orm

#endif // VIX_ERRORS_HPP
