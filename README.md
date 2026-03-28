# Vix ORM

<p align="center">
  <strong>A thin, explicit ORM layer for modern C++</strong><br/>
  Structure your data access without losing control
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Status-Active-success" />
</p>

---

## What is Vix ORM?

**Vix ORM** is a lightweight object-mapping layer built on top of **Vix DB**.

It gives you just enough structure to:
- organize your domain models
- centralize database access
- write safer, clearer code

Without:
- hiding SQL
- introducing runtime magic
- sacrificing performance

> It is not here to replace SQL.
> It is here to make your code easier to reason about.

---

## Core idea

Most ORMs try to replace the database.

Vix ORM does the opposite.

> **The database stays visible.
> The cost stays predictable.
> The control stays in your hands.**

You always know:
- when a query runs
- which connection is used
- which transaction is active
- what the SQL actually does

No surprises.

---

## Why Vix ORM exists

Traditional ORMs often introduce:
- hidden queries
- implicit transactions
- unpredictable performance
- hard-to-debug abstractions

Vix ORM removes all of that.

It is built around one principle:

> **Make data access explicit, not magical.**

---

## What Vix ORM provides

Vix ORM focuses on a small set of primitives:

### 1. Entity

A minimal base for identity-aware objects.

```cpp
struct User : vix::orm::Entity
{
  std::int64_t userId{};
  std::string name;

  std::int64_t id() const noexcept override { return userId; }
  void setId(std::int64_t v) noexcept override { userId = v; }
};
```

### 2. Mapper

Explicit mapping between database rows and objects.

```cpp
template <>
struct vix::orm::Mapper<User>
{
  static User fromRow(const vix::db::ResultRow &row)
  {
    User u{};
    u.setId(row.getInt64Or(0, 0));
    u.name = row.getStringOr(1, "");
    return u;
  }

  static vix::orm::FieldValues toInsertFields(const User &u)
  {
    return {{"name", u.name}};
  }

  static vix::orm::FieldValues toUpdateFields(const User &u)
  {
    return {{"name", u.name}};
  }
};
```

No reflection. No macros. Fully controlled.

### 3. Repository

A thin, explicit CRUD layer.

```cpp
auto repo = vix::orm::repository<User>(db, "users");

User user{};
user.name = "Alice";

auto id = repo.create(user);
auto result = repo.findById(static_cast<std::int64_t>(id));
```

### 4. Unit of Work

Explicit transaction boundaries.

```cpp
auto uow = vix::orm::unit_of_work(db);

// run multiple operations here

uow.commit();
```

No implicit transactions. Ever.

---

## What Vix ORM does NOT do

Vix ORM is intentionally limited.

It does NOT:
- generate schemas
- hide SQL behind a DSL
- perform lazy loading
- track dirty state automatically
- use runtime reflection

If you need those features, this is not the right tool.

---

## Design philosophy

Vix ORM is built around 5 rules:

1. **No hidden queries**
2. **No implicit transactions**
3. **No runtime magic**
4. **SQL stays first-class**
5. **Performance is predictable**

Everything else is optional.

---

## Built on top of Vix DB

Vix ORM uses **Vix DB** as its foundation.

That means:
- explicit connection pooling
- deterministic transaction handling
- direct access to drivers like SQLite and MySQL
- zero duplication of database logic

If needed, you can drop to raw DB instantly:

```cpp
auto conn = db.pool().acquire();
auto st = conn->prepare("SELECT * FROM users");
auto rs = st->query();
```

No lock-in.

---

## When to use Vix ORM

Use it if you want:
- structure without abstraction overhead
- explicit data access patterns
- predictable performance
- long-term maintainable C++ systems

Do not use it if you want:
- full automation
- rapid prototyping with heavy abstraction
- Rails-style magic

---

## Examples

The module includes a full set of examples:

```text
examples/
├── 01_basic_repository.cpp
├── 02_repository_crud.cpp
├── 03_find_all_and_count.cpp
├── 04_exists_and_delete.cpp
├── 05_unit_of_work.cpp
├── 06_batch_insert_tx.cpp
├── 07_query_builder_select.cpp
├── 08_query_builder_update.cpp
├── 09_error_handling.cpp
├── 10_custom_repository.cpp
├── 11_sqlite_repository.cpp
├── 12_mysql_repository.cpp
├── 13_migrations_code.cpp
├── 14_migrations_files.cpp
├── 15_entity_identity.cpp
└── 16_manual_sql_with_orm.cpp
```

Build them:

```bash
vix build -- -DVIX_ORM_BUILD_EXAMPLES=ON
```

---

## Modern C++ only

Vix ORM is designed for modern systems:
- C++20
- RAII everywhere
- explicit ownership
- no macros
- no code generation

Everything is visible in the code.

---

## Part of Vix.cpp

Vix ORM is an optional module of **Vix.cpp**.

It integrates with:
- Vix Core
- Vix DB
- Vix CLI
- networking modules

Use only what you need.

---

## Getting started

Clone the main repository:

```bash
git clone https://github.com/vixcpp/vix.git
cd vix/modules/orm
```

Build the module:

```bash
vix build
```

Build with examples:

```bash
vix build -- -DVIX_ORM_BUILD_EXAMPLES=ON
```

---

## Philosophy in one sentence

> Structure your data access.
> Keep your control.
> Never hide the cost.

---

## Support

If you believe backend tools should be:
- explicit
- predictable
- performance-aware

consider starring the project.

---

MIT License

