#ifndef VIX_ENTITY_HPP
#define VIX_ENTITY_HPP

#include <cstdint>
#include <string>

/**
 * @file Entity.hpp
 * @brief Base marker type for ORM domain models in Vix.
 *
 * The `Entity` struct is a **lightweight marker base class** that your domain
 * models can inherit from to indicate they participate in the ORM layer.
 * It does **not** impose any ID field, virtual methods, or storage policy.
 *
 * ---
 * ## When to inherit from `Entity`
 * - You want the type to be handled by generic repository helpers.
 * - You want to keep your domain models clearly separated from DTOs or view models.
 * - You plan to provide a `Mapper<T>` specialization for this type.
 *
 * Inheriting from `Entity` is **optional**; Vix ORM works with any `T` that has
 * a `Mapper<T>` specialization. Using `Entity` is recommended for clarity and tooling.
 *
 * ---
 * ## Typical usage
 * ```cpp
 * #include <vix/orm/orm.hpp>
 *
 * struct User : public Vix::orm::Entity {
 *   std::int64_t id{};
 *   std::string  name;
 *   std::string  email;
 *   int          age{};
 * };
 *
 * namespace Vix::orm {
 * template<> struct Mapper<User> {
 *   static std::vector<std::pair<std::string, std::any>> toInsertParams(const User& u) {
 *     return { {"name", u.name}, {"email", u.email}, {"age", u.age} };
 *   }
 *   static std::vector<std::pair<std::string, std::any>> toUpdateParams(const User& u) {
 *     return { {"name", u.name}, {"email", u.email}, {"age", u.age} };
 *   }
 *   // Optionally, when ResultSet adapter is available:
 *   // static User fromRow(const ResultRow& r) { ... }
 * };
 * } // namespace Vix::orm
 *
 * // Usage with a repository
 * // ConnectionPool pool{...};
 * // BaseRepository<User> users{pool, "users"};
 * // auto id = users.create(User{0, "Alice", "alice@example.com", 28});
 * ```
 *
 * ---
 * ## Design notes
 * - This class is intentionally **empty** (marker) to keep your domain models
 *   free of ORM concerns (no forced `id`, no intrusive interface).
 * - Identity strategy (e.g., `id` field type, composite keys) is **up to you** and
 *   implemented via `Mapper<T>` and repository methods.
 * - Serialization, validation, and invariants live in your domain layer; the ORM
 *   is responsible for persistence only.
 *
 * @see Mapper.hpp
 * @see Repository.hpp
 */
namespace Vix::orm
{
    /**
     * @brief Marker base class for ORM-aware domain entities.
     *
     * Inherit from `Entity` to signal that a type is an ORM domain model.
     * No virtual members beyond the destructor; no required fields.
     */
    struct Entity
    {
        virtual ~Entity() = default;
    };
} // namespace Vix::orm

#endif // VIX_ENTITY_HPP
