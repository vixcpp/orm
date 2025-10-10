/**
 * @file Repository.cpp
 * @brief Contributor documentation — internals & guidelines for BaseRepository<T>.
 *
 * This translation unit intentionally defines no linker-visible symbols.
 * It documents design choices, invariants, and recommended patterns for
 * contributors evolving the repository layer of Vix ORM.
 *
 * ───────────────────────────────────────────────────────────────────────────
 *  Contents
 * ───────────────────────────────────────────────────────────────────────────
 *  1) Responsibilities & non-responsibilities
 *  2) SQL generation model (INSERT / UPDATE / DELETE / SELECT)
 *  3) Safety & correctness (params ordering, SQL injection)
 *  4) Transactions & error handling
 *  5) Performance considerations
 *  6) SELECT roadmap (ResultSet/ResultRow integration)
 *  7) Extensions (filters, pagination, sorting, upsert)
 *
 * ----------------------------------------------------------------------------
 * 1) Responsibilities & non-responsibilities
 * ----------------------------------------------------------------------------
 *  BaseRepository<T> owns the "CRUD boilerplate":
 *    - Compose parameterized SQL for common operations.
 *    - Bind parameters in positional order (1-based).
 *    - Execute through a pooled connection (RAII via PooledConn).
 *    - Delegate type mapping to Mapper<T>.
 *
 *  It deliberately does NOT:
 *    - Perform business logic or validation.
 *    - Escape identifiers (table/column names must be trusted).
 *    - Construct dynamic WHERE clauses beyond simple cases.
 *    - Manage transactions across multiple repository calls.
 *
 * ----------------------------------------------------------------------------
 * 2) SQL generation model
 * ----------------------------------------------------------------------------
 *  INSERT:
 *    - Build a column list and matching "?, ?, ?" placeholder list from
 *      Mapper<T>::toInsertParams(v).
 *    - Bind in the exact order returned by the mapper.
 *    - Exec and then fetch lastInsertId() from the connection.
 *
 *  UPDATE:
 *    - Build "SET col1=?, col2=?, ..." from Mapper<T>::toUpdateParams(v).
 *    - Bind values in the same order, then append the WHERE id=? parameter.
 *
 *  DELETE:
 *    - Fixed statement "DELETE FROM <table> WHERE id=?"
 *
 *  SELECT (WIP):
 *    - Current code returns std::nullopt; final version will:
 *        * prepare "SELECT * FROM <table> WHERE id=? LIMIT 1"
 *        * bind id, run query()
 *        * if (rs->next()) return Mapper<T>::fromRow(*rs->row())
 *        * else return std::nullopt
 *
 * ----------------------------------------------------------------------------
 * 3) Safety & correctness
 * ----------------------------------------------------------------------------
 *  - **SQL injection**:
 *      All user-controlled *values* must go through placeholders (bind()).
 *      Table and column names are assumed trusted (owned by application).
 *
 *  - **Parameter ordering**:
 *      The order in Mapper<T>::toInsertParams / toUpdateParams defines the
 *      binding order. Keep it deterministic and documented per entity.
 *
 *  - **Types**:
 *      Mapper<T> should expose std::any values of supported driver types
 *      (int, int64_t, unsigned, double, float, bool, const char*, string).
 *      The driver throws DBError on unsupported types.
 *
 * ----------------------------------------------------------------------------
 * 4) Transactions & error handling
 * ----------------------------------------------------------------------------
 *  - The repository methods shown do not open transactions by themselves.
 *    Callers should use Transaction RAII or begin()/commit()/rollback() on
 *    Connection if atomicity across multiple repo calls is required.
 *
 *  - Exceptions:
 *      * Driver failures → DBError
 *      * Transaction boundary issues → TxnError (thrown by higher layers)
 *      * Absence (read) → std::optional<T> (prefer) or NotFound when absence
 *        is exceptional (service/controller layer choice).
 *
 * ----------------------------------------------------------------------------
 * 5) Performance considerations
 * ----------------------------------------------------------------------------
 *  - Avoid building strings repeatedly in hot paths; reserve() when possible.
 *  - Keep Mapper<T>::to*Params vectors small and pre-reserved.
 *  - Use ConnectionPool to amortize connection setup.
 *  - Consider driver-side statement caching in future iterations.
 *
 * ----------------------------------------------------------------------------
 * 6) SELECT roadmap (ResultSet/ResultRow)
 * ----------------------------------------------------------------------------
 *  Once the MySQL ResultSet adapter is implemented:
 *
 *  @code
 *  std::optional<T> BaseRepository<T>::findById(std::int64_t id) {
 *      PooledConn pc(pool_);
 *      auto st = pc.get().prepare("SELECT <cols> FROM " + table_ + " WHERE id=? LIMIT 1");
 *      st->bind(1, id);
 *      auto rs = st->query();
 *      if (rs->next()) {
 *          auto r = rs->row();
 *          return Mapper<T>::fromRow(*r);
 *      }
 *      return std::nullopt;
 *  }
 *  @endcode
 *
 *  Column selection:
 *    - Prefer explicit column lists matching Mapper<T>::fromRow order for clarity.
 *    - Wildcard (*) is acceptable in simple tables but less future-proof.
 *
 * ----------------------------------------------------------------------------
 * 7) Extensions
 * ----------------------------------------------------------------------------
 *  - Filters:
 *      Provide dedicated methods for common predicates (by_email, by_status).
 *      Use QueryBuilder to compose WHERE clauses safely with placeholders.
 *
 *  - Pagination:
 *      Expose (limit, offset) with strict bounds and defaults; bind as params:
 *        "... ORDER BY id DESC LIMIT ? OFFSET ?"
 *
 *  - Sorting:
 *      Never interpolate user-supplied column names directly. Use whitelists:
 *        assert_in({"id","name","created_at"}, user_col);
 *
 *  - Upsert:
 *      Dialect helpers: MySQL (ON DUPLICATE KEY UPDATE), PostgreSQL (ON CONFLICT).
 *      Keep it in a separate API to avoid leaking dialect rules into base repo.
 *
 *  - Batch operations:
 *      Consider `create_many(std::span<const T>)` with batched exec or multi-values
 *      INSERT where supported, or driver-agnostic loop otherwise.
 */

#include <vix/orm/Repository.hpp>

// This TU intentionally contains no definitions.
// The BaseRepository<T> implementation is in the header (templates).
//
// If internal helpers are later needed (e.g., identifier whitelists,
// column list caches, or statement caches), they can be placed here or
// in a private header included by this TU and the repository header.
