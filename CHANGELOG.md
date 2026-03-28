# Changelog

All notable changes to **Vix ORM** are documented in this file.

This project follows a developer-first approach:
- explicit changes
- no hidden behavior
- predictable evolution

---

## [1.9.0] - 2026-03-28

### Highlights

This release focuses on **clarity, correctness, and developer experience**.

- Full rewrite of examples
- API consistency improvements
- Fixes for move semantics and const correctness
- Cleaner positioning of ORM as a **pure explicit layer on top of Vix DB**

---

### Added

- Complete structured examples suite (16 examples):

  - basic repository usage
  - full CRUD flows
  - unit of work
  - batch operations
  - query builder usage
  - error handling
  - custom repositories
  - SQLite / MySQL integration
  - migrations (code + files)
  - entity identity handling
  - manual SQL integration

- `examples/CMakeLists.txt`:
  - automatic registration of examples
  - unified build output
  - migration files auto-copy

---

### Changed

- **README.md**
  - rewritten for clarity and positioning
  - stronger “explicit ORM” philosophy
  - real-world usage examples added

- **ORM API stabilization**

  - `Entity`
    - clarified identity model

  - `Mapper`
    - standardized:
      - `toInsertFields`
      - `toUpdateFields`

  - `Repository`
    - improved SQL generation clarity
    - stricter behavior on empty fields

  - `QueryBuilder`
    - safer parameter handling
    - improved API consistency

  - `db_compat.hpp`
    - centralized DB interop layer
    - cleaner type exposure

- **CMake**
  - improved example integration
  - better standalone + monorepo compatibility

---

### Removed

- Legacy examples:
  - batch_insert_tx.cpp
  - error_handling.cpp
  - querybuilder_update.cpp
  - repository_crud_full.cpp
  - tx_unit_of_work.cpp
  - users_crud.cpp

Replaced by a structured numbered examples system.

---

### Fixed

- **UnitOfWork**

  - removed invalid move assignment
    (Transaction is non-assignable)

  - fixed const correctness for `conn()`

- **Examples**

  - fixed entity initialization issues
  - aligned examples with current API

- **General**

  - improved build stability for examples
  - fixed multiple compilation edge cases

---

## [1.8.0]

### Added
- `vix.json` module definition for `@vix/orm`
- improved packaging (header + source module)

---

## [1.7.x]

### Changed
- standalone build improvements
- fetch only required modules (`core`, `db`)

### Fixed
- CI stability
- CMake standalone issues

---

## [1.6.0]

### Changed
- stabilized public API
- improved examples
- inline templates

---

## [1.5.0]

### Changed
- internal refactor

---

## [1.4.x]

### Added
- file-based migrations runner
- CLI migration support

### Fixed
- MySQL driver gating
- connection abstraction issues

---

## [1.3.0]

### Fixed
- CMake integration
- MySQL linking issues

---

## [1.2.x]

### Added
- `Database` abstraction
- multi-engine support (MySQL / SQLite)

---

## [1.1.x]

### Changed
- modern CMake
- MySQL / SQLite detection
- sanitizers
- documentation improvements

---

## [1.0.0]

### Added
Initial release:

- ConnectionPool
- Transaction system
- UnitOfWork
- MySQL driver
- modular CMake
- documentation + examples

---

## Philosophy

Vix ORM evolves with one invariant:

> No magic
> No hidden cost
> No abstraction leaks
