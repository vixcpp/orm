/**
 *
 *  @file Migration.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_MIGRATION_HPP
#define VIX_MIGRATION_HPP

#include <string>
#include <vix/orm/Drivers.hpp>

namespace vix::orm
{
  struct Migration
  {
    virtual ~Migration() = default;
    virtual std::string id() const = 0;
    virtual void up(Connection &c) = 0;
    virtual void down(Connection &c) = 0;
  };
} // namespace Vix::orm

#endif // VIX_MIGRATION_HPP
