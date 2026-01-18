/**
 *
 *  @file MigratorCLI.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include "MigratorCLI.hpp"

#include <vix/orm/orm.hpp>
#include <vix/orm/FileMigrationsRunner.hpp>
#include <vix/orm/Drivers.hpp>

#include <iostream>
#include <stdexcept>
#include <memory>

namespace vix::orm::tools
{
  void MigratorCLI::printUsage(const char *prog)
  {
    std::cout
        << "Vix ORM Migrator\n\n"
        << "Usage:\n"
        << "  " << prog << " <host> <user> <pass> <db> migrate   [--dir <migrations_dir>]\n"
        << "  " << prog << " <host> <user> <pass> <db> rollback  --steps <n> [--dir <migrations_dir>]\n"
        << "  " << prog << " <host> <user> <pass> <db> status    [--dir <migrations_dir>]\n\n"
        << "Examples:\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb migrate --dir ./migrations\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb rollback --steps 1\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb status --dir db/migrations\n";
  }

  std::string MigratorCLI::getFlagValue(
      const std::vector<std::string> &args,
      const std::string &key)
  {
    for (size_t i = 0; i + 1 < args.size(); ++i)
    {
      if (args[i] == key)
        return args[i + 1];
    }
    return {};
  }

  bool MigratorCLI::hasFlag(
      const std::vector<std::string> &args,
      const std::string &key)
  {
    for (const auto &a : args)
    {
      if (a == key)
        return true;
    }
    return false;
  }

  std::string MigratorCLI::parseDir(const std::vector<std::string> &args)
  {
    std::string v = getFlagValue(args, "--dir");
    if (!v.empty())
      return v;

    for (const auto &a : args)
    {
      const std::string p = "--dir=";
      if (a.rfind(p, 0) == 0)
      {
        std::string vv = a.substr(p.size());
        if (!vv.empty())
          return vv;
      }
    }

    return "migrations";
  }

  int MigratorCLI::parseStepsOrThrow(const std::vector<std::string> &args)
  {
    std::string steps_s = getFlagValue(args, "--steps");
    if (steps_s.empty())
      throw std::runtime_error("rollback requires --steps <n>");

    int steps = 0;
    try
    {
      steps = std::stoi(steps_s);
    }
    catch (...)
    {
      throw std::runtime_error("invalid --steps value (must be an integer)");
    }

    if (steps <= 0)
      throw std::runtime_error("--steps must be >= 1");

    return steps;
  }

  void MigratorCLI::validateOrThrow(const Options &opt)
  {
    if (opt.help)
      return;

    if (opt.host.empty() || opt.user.empty() || opt.db.empty())
      throw std::runtime_error("missing required args: <host> <user> <pass> <db> <command>");

    if (opt.command != "migrate" && opt.command != "rollback" && opt.command != "status")
      throw std::runtime_error("unknown command: " + opt.command);

    if (opt.command == "rollback" && opt.steps <= 0)
      throw std::runtime_error("rollback requires --steps <n>");

    if (opt.migrationsDir.empty())
      throw std::runtime_error("migrations dir is empty (use --dir <path>)");
  }

  MigratorCLI::Options MigratorCLI::parseArgsOrThrow(int argc, char **argv)
  {
    Options opt;

    std::vector<std::string> all;
    all.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
      all.emplace_back(argv[i]);

    if (argc <= 1 || hasFlag(all, "-h") || hasFlag(all, "--help"))
    {
      opt.help = true;
      return opt;
    }

    if (argc < 6)
      throw std::runtime_error("not enough arguments");

    opt.host = argv[1];
    opt.user = argv[2];
    opt.pass = argv[3];
    opt.db = argv[4];
    opt.command = argv[5];

    std::vector<std::string> extra;
    for (int i = 6; i < argc; ++i)
      extra.emplace_back(argv[i]);

    opt.migrationsDir = parseDir(extra);

    if (opt.command == "rollback")
      opt.steps = parseStepsOrThrow(extra);

    validateOrThrow(opt);
    return opt;
  }

  int MigratorCLI::run(int argc, char **argv)
  {
    try
    {
      Options opt = parseArgsOrThrow(argc, argv);

      if (opt.help)
      {
        printUsage(argv[0]);
        return 0;
      }

      vix::orm::ConnectionPtr conn;
      std::unique_ptr<vix::orm::FileMigrationsRunner> runner;

#if VIX_ORM_HAS_MYSQL
      {
        auto factory = vix::orm::make_mysql_factory(opt.host, opt.user, opt.pass, opt.db);
        conn = factory();
        runner = std::make_unique<vix::orm::FileMigrationsRunner>(*conn, opt.migrationsDir);
      }
#elif VIX_ORM_HAS_SQLITE
      {
        auto factory = vix::orm::make_sqlite_factory(opt.db);
        conn = factory();
        runner = std::make_unique<vix::orm::FileMigrationsRunner>(*conn, opt.migrationsDir);
      }
#else
      std::cerr << "[ERR] vix_orm_migrator built without DB drivers.\n"
                << "Enable one with:\n"
                << "  -DVIX_ORM_HAS_MYSQL=1 (or add SQLite support)\n";
      return 1;
#endif

      if (opt.command == "migrate")
      {
        runner->applyAll();
        std::cout << "[OK] migrations applied\n";
        return 0;
      }

      if (opt.command == "rollback")
      {
        runner->rollback(opt.steps);
        std::cout << "[OK] rollback " << opt.steps << " step(s)\n";
        return 0;
      }

      if (opt.command == "status")
      {
        std::cout << "[OK] migrations dir: " << opt.migrationsDir << "\n";
        std::cout << "Tip: implement FileMigrationsRunner::status() to show applied vs pending.\n";
        return 0;
      }

      printUsage(argv[0]);
      return 1;
    }
    catch (const std::exception &e)
    {
      std::cerr << "[ERR] " << e.what() << "\n";
      std::cerr << "Tip: run with --help\n";
      return 1;
    }
  }

} // namespace vix::orm::tools
