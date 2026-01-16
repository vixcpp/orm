/**
 *
 *  @file main.cpp
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

int main(int argc, char **argv)
{
  return vix::orm::tools::MigratorCLI::run(argc, argv);
}
