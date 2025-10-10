/**
 * @file Mapper.cpp
 * @brief Contributor-oriented documentation & guidelines for Vix ORM mappers.
 *
 * This translation unit intentionally contains **no linker-visible symbols**.
 * Its purpose is to document design choices, contracts, and best practices
 * for authors of `Mapper<T>` specializations and contributors to the ORM.
 *
 *  ──────────────────────────────────────────────────────────────────────────
 *  Contents
 *  ──────────────────────────────────────────────────────────────────────────
 *  1) What a Mapper does (and does not)
 *  2) Column indexing & nullability rules
 *  3) Supported parameter types (std::any → driver bindings)
 *  4) Error handling & invariants
 *  5) Performance tips
 *  6) Examples
 *  7) Future roadmap / open questions
 *
 *  -------------------------------------------------------------------------
 *  1) What a Mapper does (and does not)
 *  -------------------------------------------------------------------------
 *  A `Mapper<T>` specialization provides 3 static functions:
 *    - T fromRow(const ResultRow&)
 *    - std::vector<std::pair<std::string, std::any>> toInsertParams(const T&)
 *    - std::vector<std::pair<std::string, std::any>> toUpdateParams(const T&)
 *
 *  Responsibilities:
 *   ✓ Define how to construct T from a DB row (SELECT results).
 *   ✓ Define insert/update parameter lists with **column names** and values.
 *   ✗ Do not perform I/O or own connections/statements.
 *   ✗ Do not build raw SQL (Repository/QueryBuilder handle that).
 *
 *  -------------------------------------------------------------------------
 *  2) Column indexing & nullability rules
 *  -------------------------------------------------------------------------
 *  - `ResultRow` accessors use **0-based** column indexing.
 *  - The order in `fromRow` MUST match the SELECT list order in the repository.
 *  - If a column can be NULL:
 *      * Either call `row.isNull(i)` and choose a default,
 *      * Or use an optional field in T and handle it at a higher layer.
 *  - Avoid silent conversions that lose information (e.g., double → int).
 *
 *  -------------------------------------------------------------------------
 *  3) Supported parameter types (std::any → driver bindings)
 *  -------------------------------------------------------------------------
 *  The driver layer binds `std::any` values by inspection of `typeid`:
 *    - int, std::int64_t, unsigned
 *    - double, float
 *    - bool
 *    - const char*, std::string
 *
 *  Tips:
 *   - Prefer exact-width ints (std::int64_t) when mapping BIGINT.
 *   - Use std::string for text; drivers will copy or reference accordingly.
 *   - For enums, map to an integral type in `toInsertParams`/`toUpdateParams`.
 *
 *  -------------------------------------------------------------------------
 *  4) Error handling & invariants
 *  -------------------------------------------------------------------------
 *  - Mappers should be **strict**: if a column is missing or has the wrong type,
 *    let the driver throw (DBError) rather than silently coercing.
 *  - `toInsertParams` should **exclude auto-increment primary keys**.
 *  - `toUpdateParams` should include only mutable columns.
 *  - Repository methods decide how to treat absence (optional<T> vs NotFound).
 *
 *  -------------------------------------------------------------------------
 *  5) Performance tips
 *  -------------------------------------------------------------------------
 *  - Keep `fromRow` trivial: read columns in order, avoid heap allocations.
 *  - Pre-reserve vectors in `toInsertParams`/`toUpdateParams` if many fields.
 *  - Reuse small strings (SBO) where applicable; avoid unnecessary copies.
 *  - Avoid constructing large temporaries in loops (hot paths).
 *
 *  -------------------------------------------------------------------------
 *  6) Examples
 *  -------------------------------------------------------------------------
 *  See `Mapper.hpp` for a user-facing example. Below is a contributor-oriented
 *  variant showing null handling and enum mapping.
 *
 *  Example:
 *  -------------------------------------------------------------------------
 *  enum class Role : int { User = 0, Admin = 1 };
 *
 *  struct Account {
 *    std::int64_t id{};
 *    std::string  email;
 *    std::optional<std::string> display_name; // may be NULL in DB
 *    Role role{Role::User};
 *  };
 *
 *  namespace Vix::orm {
 *  template <> struct Mapper<Account> {
 *    static Account fromRow(const ResultRow& r) {
 *      Account a{};
 *      a.id    = r.getInt64(0);
 *      a.email = r.getString(1);
 *      if (!r.isNull(2)) a.display_name = r.getString(2);
 *      a.role  = static_cast<Role>(r.getInt64(3)); // stored as INT in DB
 *      return a;
 *    }
 *    static std::vector<std::pair<std::string, std::any>>
 *    toInsertParams(const Account& a) {
 *      std::vector<std::pair<std::string, std::any>> v;
 *      v.reserve(3 + (a.display_name ? 1 : 0));
 *      v.emplace_back("email", a.email);
 *      if (a.display_name) v.emplace_back("display_name", *a.display_name);
 *      v.emplace_back("role", static_cast<int>(a.role));
 *      return v;
 *    }
 *    static std::vector<std::pair<std::string, std::any>>
 *    toUpdateParams(const Account& a) {
 *      std::vector<std::pair<std::string, std::any>> v;
 *      v.reserve(2 + (a.display_name ? 1 : 0));
 *      if (a.display_name) v.emplace_back("display_name", *a.display_name);
 *      v.emplace_back("role", static_cast<int>(a.role));
 *      return v;
 *    }
 *  };
 *  } // namespace Vix::orm
 *
 *  -------------------------------------------------------------------------
 *  7) Future roadmap / open questions
 *  -------------------------------------------------------------------------
 *  - Column-by-name accessors:
 *      We may extend `ResultRow` with `getString(std::string_view col)`, etc.
 *      This needs driver support for per-column name lookup with minimal overhead.
 *
 *  - Compile-time reflection:
 *      We could offer an opt-in adapter using C++20 reflection once available,
 *      or external libraries (magic_get/visit_struct) to generate mappers.
 *
 *  - Custom converters:
 *      Provide a trait-based mechanism to convert/proxy types (e.g., chrono dates,
 *      UUIDs, decimal/fixed-precision types).
 *
 *  - Null policy:
 *      Consider a `Nullable<T>` helper to unify null handling and defaults.
 *
 *  - Batch mapping helpers:
 *      Convenience functions to stream a whole `ResultSet` into `std::vector<T>`
 *      using `Mapper<T>::fromRow` once the `ResultSet` adapter lands.
 */

#include <vix/orm/Mapper.hpp>

// This TU intentionally contains no definitions.
// Mapper specializations must live in headers to avoid ODR/linker issues.

// Optionally, keep internal helpers in an anonymous namespace behind a macro.
// They are commented out to avoid exposing private contracts.

/*
namespace {
    // Example: sanitizer for std::any types accepted by drivers
    inline bool is_supported_any_type(const std::any& a) noexcept {
        return a.type() == typeid(int) ||
               a.type() == typeid(std::int64_t) ||
               a.type() == typeid(unsigned) ||
               a.type() == typeid(double) ||
               a.type() == typeid(float) ||
               a.type() == typeid(bool) ||
               a.type() == typeid(const char*) ||
               a.type() == typeid(std::string);
    }
}
*/
