#ifndef VIX_MIGRATIONS_RUNNER_HPP
#define VIX_MIGRATIONS_RUNNER_HPP

#include <vix/orm/Migration.hpp>
#include <vector>

namespace vix::orm
{
    class MigrationsRunner
    {
        Connection &conn_;
        std::vector<Migration *> migs_;

    public:
        explicit MigrationsRunner(Connection &c) : conn_(c) {}
        void add(Migration *m) { migs_.push_back(m); }
        void runAll();
    };
} // namespace Vix::orm

#endif // VIX_MIGRATIONS_RUNNER_HPP
