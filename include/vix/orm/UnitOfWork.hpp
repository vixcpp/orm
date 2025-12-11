#ifndef VIX_UNIT_OF_WORK_HPP
#define VIX_UNIT_OF_WORK_HPP

#include <vix/orm/Transaction.hpp>

namespace vix::orm
{
    class UnitOfWork
    {
        Transaction tx_;

    public:
        explicit UnitOfWork(ConnectionPool &pool) : tx_(pool) {}
        void commit() { tx_.commit(); }
        void rollback() { tx_.rollback(); }
        Connection &conn() { return tx_.conn(); }
    };
} // namespace Vix::orm

#endif // VIX_UNIT_OF_WORK_HPP
