# Vix ORM

<p align="center">
  <strong>A thin, explicit ORM layer for modern C++</strong><br/>
  No magic · No hidden queries · No performance tax
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Status-Active-success" />
</p>

---

## What is Vix ORM?

**Vix ORM** is the optional object-mapping layer of **Vix.cpp**.

It provides a **lightweight, intentional abstraction** on top of **Vix DB**
for developers who want:

- cleaner domain models
- structured repositories
- safer data access patterns

…without giving up **control, predictability, or performance**.

Vix ORM is not designed to hide the database.
It is designed to **organize your code**, not obscure what happens underneath.

---

## Why another ORM?

Most ORMs today:

- try to abstract SQL completely
- generate complex, unpredictable queries
- hide transactions and connections
- make performance issues hard to diagnose

Vix ORM takes a different path.

> **SQL is still there.
> The database is still visible.
> The cost model stays explicit.**

You always know:
- when a query runs
- inside which transaction
- on which connection
- with which performance implications

---

## A “sugar layer”, not a framework

Vix ORM is intentionally **thin**.

It focuses on:
- mapping rows to objects
- organizing queries in repositories
- grouping operations with unit-of-work patterns

It does **not**:
- invent its own query language
- auto-generate schemas
- introduce runtime reflection
- enforce a specific architecture

You stay in control.

---

## Built on top of Vix DB

Vix ORM is powered by **Vix DB**, the low-level database layer of Vix.cpp.

That means:
- explicit transactions
- predictable pooling behavior
- no hidden drivers
- no duplicate database logic

If you ever outgrow the ORM,
you can drop down to Vix DB **without rewriting your application**.

---

## When should you use Vix ORM?

Vix ORM is a good fit if you:

- want structure without magic
- prefer repositories over raw SQL everywhere
- value performance and debuggability
- build long-lived C++ systems
- dislike “active record everywhere” patterns

If you want full control, you can skip it.
If you want light structure, it’s there.

---

## Modern C++ by design

Vix ORM is built with modern C++ principles:

- C++20
- explicit ownership
- RAII for safety
- compile-time clarity
- minimal abstractions

No macros.
No code generation.
No runtime surprises.

---

## Part of the Vix.cpp ecosystem

Vix ORM is an **optional module** of **Vix.cpp**.

It integrates seamlessly with:
- Vix Core
- Vix DB
- Vix CLI
- WebSocket & network modules

Use only what you need.

---

## Getting started

Vix ORM is built automatically when enabled through the Vix.cpp umbrella.

For setup, examples, and usage, start from the main repository:

👉 https://github.com/vixcpp/vix

---

## ⭐ Support the project

If you believe ORMs should be:
- explicit
- predictable
- performance-aware

please consider starring **Vix.cpp**.

MIT License
