#pragma once

#include <vix/db/Drivers.hpp>
#include <vix/db/Errors.hpp>
#include <vix/db/ConnectionPool.hpp>
#include <vix/db/Transaction.hpp>
#include <vix/db/Database.hpp>
#include <vix/db/Migration.hpp>
#include <vix/db/FileMigrationsRunner.hpp>
#include <vix/db/MigrationsRunner.hpp>
#include <vix/db/Sha256.hpp>
#include <vix/db/mysql/MySQLDriver.hpp>

namespace vix::orm
{
  // Temporary compatibility aliases.
  // Keep for 1-2 versions, then remove once downstream code migrates to vix::db.
  using vix::db::ResultRow;
  using vix::db::ResultSet;
  using vix::db::Statement;
  using vix::db::Connection;
  using vix::db::ConnectionPtr;
  using vix::db::ConnectionFactory;

  using vix::db::PoolConfig;
  using vix::db::ConnectionPool;
  using vix::db::PooledConn;

  using vix::db::Transaction;
  using vix::db::Database;

  using vix::db::Migration;
  using vix::db::FileMigrationsRunner;
  using vix::db::MigrationsRunner;

  using vix::db::Sha256;
}
