/**
 *
 *  @file FileMigrationRunner.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_FILE_MIGRATIONS_RUNNER_HPP
#define VIX_FILE_MIGRATIONS_RUNNER_HPP

#include <vix/orm/Drivers.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace vix::orm
{
  struct MigrationPair
  {
    std::string id; // base id (no .up/.down)
    std::filesystem::path up_path;
    std::filesystem::path down_path; // can be empty if missing
    std::string up_checksum;         // sha256(up.sql content)
  };

  class FileMigrationsRunner
  {
    Connection &conn_;
    std::filesystem::path dir_;
    std::string table_ = "schema_migrations";

  public:
    FileMigrationsRunner(Connection &c, std::filesystem::path migrationsDir)
        : conn_(c), dir_(std::move(migrationsDir)) {}

    void setTable(std::string t) { table_ = std::move(t); }

    void applyAll();          // apply pending .up.sql
    void rollback(int steps); // rollback last N applied (requires .down.sql)

  private:
    void ensureTable();
    std::vector<MigrationPair> scanPairs() const;

    bool isApplied(const std::string &id, std::string *checksum_out = nullptr);
    void markApplied(const std::string &id, const std::string &checksum);
    void unmarkApplied(const std::string &id);

    std::string lastAppliedId(); // ORDER BY id DESC (timestamp prefix)

    static std::string readFileText(const std::filesystem::path &p);
    static std::string trim(std::string s);
    static std::vector<std::string> splitStatements(const std::string &sql);
    void execScript(const std::string &sql);
  };
}

#endif
