/**
 *
 *  @file orm.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */

#ifndef VIX_ORM_HPP
#define VIX_ORM_HPP

#include <vix/orm/Errors.hpp>
#include <vix/orm/Drivers.hpp>
#if VIX_ORM_HAS_MYSQL
#include <vix/orm/MySQLDriver.hpp>
#endif
#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/Transaction.hpp>
#include <vix/orm/QueryBuilder.hpp>
#include <vix/orm/Entity.hpp>
#include <vix/orm/Mapper.hpp>
#include <vix/orm/Repository.hpp>
#include <vix/orm/UnitOfWork.hpp>
#include <vix/orm/Migration.hpp>
#include <vix/orm/MigrationsRunner.hpp>

#endif
