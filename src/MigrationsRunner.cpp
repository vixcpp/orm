#include <vix/orm/MigrationsRunner.hpp>

namespace vix::orm
{
    void MigrationsRunner::runAll()
    {
        for (auto *m : migs_)
        {
            m->up(conn_);
        }
    }
} // namespace Vix::orm
