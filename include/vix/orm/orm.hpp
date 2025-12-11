// ==========================================================
// Vix.cpp ORM — Umbrella Header
// ==========================================================
// Import this header to access the entire ORM subsystem
// instead of including individual headers manually.
// ==========================================================

#pragma once

#include <vix/orm/Errors.hpp>
#include <vix/orm/Drivers.hpp>
#include <vix/orm/MySQLDriver.hpp>
#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/Transaction.hpp>
#include <vix/orm/QueryBuilder.hpp>
#include <vix/orm/Entity.hpp>
#include <vix/orm/Mapper.hpp>
#include <vix/orm/Repository.hpp>
#include <vix/orm/UnitOfWork.hpp>
#include <vix/orm/Migration.hpp>
#include <vix/orm/MigrationsRunner.hpp>

namespace vix::orm
{
    inline void init()
    {
    }
}
