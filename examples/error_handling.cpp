/**
 *
 *  @file error_handling.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/orm/orm.hpp>
#include <iostream>

using namespace vix::orm;

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  try
  {
    // Intentionally wrong DB name to show error
    auto factory = make_mysql_factory("tcp://127.0.0.1:3306", "root", "", "db_does_not_exist");
    PoolConfig cfg;
    cfg.min = 1;
    cfg.max = 8;

    ConnectionPool pool{factory, cfg};
    pool.warmup();

    UnitOfWork uow{pool};
    auto &con = uow.conn();

    auto st = con.prepare("SELECT 1");
    st->exec();
    std::cout << "[INFO] This message may not be reached if connection fails.\n";
  }
  catch (const DBError &e)
  {
    std::cerr << "[DBError] " << e.what() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cerr << "[std::exception] " << e.what() << "\n";
  }
  return 0;
}
