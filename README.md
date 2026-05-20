# Vix.cpp ORM Module

Thin and explicit ORM layer for Vix.cpp.

The ORM module provides a lightweight object-mapping layer built on top of Vix DB. It helps structure data access with entities, mappers, repositories, query helpers, and explicit unit-of-work transactions without hiding SQL or database behavior.

## Documentation

Full documentation will be available here:

https://docs.vixcpp.com/modules/orm/

API reference:

https://docs.vixcpp.com/modules/orm/api-reference

## What ORM provides

- Entity base type
- Explicit row-to-object mapping
- Repository pattern
- CRUD helpers
- Query helpers
- Unit of Work
- Explicit transaction boundaries
- Integration with Vix DB
- SQLite support through Vix DB
- MySQL support through Vix DB
- Manual SQL escape path when needed

## Public header

```cpp
#include <vix/orm.hpp>
```

For database access:

```cpp
#include <vix/db.hpp>
#include <vix/orm.hpp>
```

## Basic entity

```cpp
#include <cstdint>
#include <string>

#include <vix/orm.hpp>

struct User : vix::orm::Entity
{
  std::int64_t userId{};
  std::string name;

  std::int64_t id() const noexcept override
  {
    return userId;
  }

  void setId(std::int64_t value) noexcept override
  {
    userId = value;
  }
};
```

## Mapper

```cpp
template <>
struct vix::orm::Mapper<User>
{
  static User fromRow(const vix::db::ResultRow &row)
  {
    User user{};

    user.setId(row.getInt64Or(0, 0));
    user.name = row.getStringOr(1, "");

    return user;
  }

  static vix::orm::FieldValues toInsertFields(const User &user)
  {
    return {
      {"name", user.name}
    };
  }

  static vix::orm::FieldValues toUpdateFields(const User &user)
  {
    return {
      {"name", user.name}
    };
  }
};
```

## Repository

```cpp
#include <vix/db.hpp>
#include <vix/orm.hpp>

int main()
{
  auto db = vix::db::Database::sqlite("vix.db");

  db.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "name TEXT NOT NULL)");

  auto repo = vix::orm::repository<User>(db, "users");

  User user{};
  user.name = "Alice";

  const auto id = repo.create(user);
  const auto result = repo.findById(static_cast<std::int64_t>(id));

  return result ? 0 : 1;
}
```

Run with SQLite support:

```bash
vix run main.cpp --with-sqlite
```

## Unit of Work

Use Unit of Work when multiple operations must share one explicit transaction.

```cpp
auto uow = vix::orm::unit_of_work(db);

// run multiple repository or database operations here

uow.commit();
```

## Manual SQL

Vix ORM does not lock you into the ORM layer.

You can always drop to raw Vix DB:

```cpp
auto conn = db.pool().acquire();

auto stmt = conn->prepare("SELECT id, name FROM users WHERE id = ?");
stmt->bind(1, static_cast<std::int64_t>(1));

auto rows = stmt->query();
```

## ORM architecture

```text
Vix DB
  -> Connection pool
  -> Repository
  -> Mapper
  -> Entity
  -> Your application
```

With Unit of Work:

```text
Database
  -> transaction
  -> Unit of Work
  -> repository operations
  -> commit or rollback
```

## Build

Contributors should use the Vix CLI to build this module.

Vix wraps the C++ build workflow with project detection, presets, Ninja builds, clean logs, caching, and focused diagnostics. This keeps the contributor workflow consistent and helps avoid hidden C++ build issues.

### Build the project

```bash
git clone https://github.com/vixcpp/vix.git
cd vix
vix build
```

### Build with ORM examples

```bash
vix build -- -DVIX_ORM_BUILD_EXAMPLES=ON
```

### Build with SQLite

```bash
vix build --with-sqlite
```

### Build with MySQL

```bash
vix build --with-mysql
```

### Build all targets

Use this before running the full test suite, install workflows, or release checks:

```bash
vix build --build-target all
```

### Clean rebuild

Use this when the local CMake cache or build directory may be stale:

```bash
vix build --clean
```

### Release build

```bash
vix build --preset release
```

## Tests

Build all targets first, then run tests:

```bash
vix build --build-target all
vix tests
```

Before opening a pull request, use:

```bash
vix fmt --check
vix build --build-target all
vix tests
```

## Useful links

- ORM documentation: https://docs.vixcpp.com/modules/orm/
- ORM API reference: https://docs.vixcpp.com/modules/orm/api-reference
- DB documentation: https://docs.vixcpp.com/book/10-database
- Build command: https://docs.vixcpp.com/cli/build
- Tests command: https://docs.vixcpp.com/cli/tests
- Documentation: https://docs.vixcpp.com/
- Engineering notes: https://blog.vixcpp.com/
- Registry: https://registry.vixcpp.com/
- GitHub: https://github.com/vixcpp/vix

## License

MIT License.

See [`LICENSE`](../../LICENSE) for details.
