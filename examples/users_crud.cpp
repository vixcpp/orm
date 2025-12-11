/**
 * @file users_crud.cpp
 * @brief Example — Basic CRUD operations with Vix ORM.
 *
 */
#include <vix/orm/orm.hpp>
#include <vix/orm/ConnectionPool.hpp>
#include <vix/orm/MySQLDriver.hpp>

#include <iostream>
#include <string>

struct User
{
    std::int64_t id{};
    std::string name;
    std::string email;
    int age{};
};

namespace vix::orm
{
    template <>
    struct Mapper<User>
    {
        static User fromRow(const ResultRow &)
        {
            return {};
        }

        static std::vector<std::pair<std::string, std::any>> toInsertParams(const User &u)
        {
            return {
                {"name", u.name},
                {"email", u.email},
                {"age", u.age},
            };
        }

        static std::vector<std::pair<std::string, std::any>> toUpdateParams(const User &u)
        {
            return {
                {"name", u.name},
                {"email", u.email},
                {"age", u.age},
            };
        }
    };
} // namespace vix::orm

int main(int argc, char **argv)
{
    using namespace vix::orm;

    std::string host = (argc > 1 ? argv[1] : "tcp://127.0.0.1:3306");
    std::string user = (argc > 2 ? argv[2] : "root");
    std::string pass = (argc > 3 ? argv[3] : "");
    std::string db = (argc > 4 ? argv[4] : "vixdb");

    try
    {
        auto factory = make_mysql_factory(host, user, pass, db);

        PoolConfig cfg;
        cfg.min = 2;
        cfg.max = 16;

        ConnectionPool pool{factory, cfg};
        pool.warmup();

        BaseRepository<User> users{pool, "users"};

        std::uint64_t id = users.create(User{
            0,
            "Gaspard",
            "gaspardkirira@outlook.com",
            28});

        std::cout << "[OK] Insert user → id=" << id << "\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERR] " << e.what() << "\n";
        return 1;
    }
}
