// #include <vix/orm/orm.hpp>
// #include <vix/orm/FileMigrationsRunner.hpp>

// #include <iostream>
// #include <string>
// #include <vector>

// using namespace vix::orm;

// static void usage(const char *prog)
// {
//     std::cout
//         << "Usage:\n"
//         << "  " << prog << " <host> <user> <pass> <db> migrate   [--dir <migrations_dir>]\n"
//         << "  " << prog << " <host> <user> <pass> <db> rollback  --steps <n> [--dir <migrations_dir>]\n"
//         << "  " << prog << " <host> <user> <pass> <db> status    [--dir <migrations_dir>]\n";
// }

// static std::string get_flag_value(const std::vector<std::string> &args, const std::string &key)
// {
//     for (size_t i = 0; i + 1 < args.size(); ++i)
//         if (args[i] == key)
//             return args[i + 1];
//     return {};
// }
// static std::string parse_dir(const std::vector<std::string> &extra)
// {
//     // 1) --dir <path>
//     std::string v = get_flag_value(extra, "--dir");
//     if (!v.empty())
//         return v;

//     // 2) --dir=<path>
//     for (const auto &a : extra)
//     {
//         const std::string p = "--dir=";
//         if (a.rfind(p, 0) == 0)
//         {
//             std::string vv = a.substr(p.size());
//             if (!vv.empty())
//                 return vv;
//         }
//     }

//     // 3) positional (premier argument qui ne commence pas par '-')
//     for (const auto &a : extra)
//     {
//         if (!a.empty() && a[0] != '-')
//             return a;
//     }

//     return "migrations";
// }

// static bool has_flag(const std::vector<std::string> &args, const std::string &key)
// {
//     for (auto &a : args)
//         if (a == key)
//             return true;
//     return false;
// }

int main(int argc, char **argv)
{
    // if (argc < 6)
    // {
    //     usage(argv[0]);
    //     return 1;
    // }

    // std::string host = argv[1];
    // std::string user = argv[2];
    // std::string pass = argv[3];
    // std::string db = argv[4];
    // std::string cmd = argv[5];

    // std::vector<std::string> extra;
    // for (int i = 6; i < argc; ++i)
    //     extra.push_back(argv[i]);

    // std::string dir = parse_dir(extra);
    // if (dir.empty())
    //     dir = "migrations";

    // try
    // {
    //     auto raw = make_mysql_conn(host, user, pass, db);
    //     MySQLConnection conn{raw};

    //     FileMigrationsRunner runner{conn, dir};

    //     if (cmd == "migrate")
    //     {
    //         runner.applyAll();
    //         std::cout << "[OK] migrations applied\n";
    //         return 0;
    //     }

    //     if (cmd == "rollback")
    //     {
    //         std::string steps_s = get_flag_value(extra, "--steps");
    //         if (steps_s.empty())
    //         {
    //             std::cerr << "[ERR] rollback requires --steps <n>\n";
    //             return 1;
    //         }
    //         int steps = std::stoi(steps_s);
    //         runner.rollback(steps);
    //         std::cout << "[OK] rollback " << steps << " step(s)\n";
    //         return 0;
    //     }

    //     if (cmd == "status")
    //     {
    //         // Minimal status (si ton runner n’a pas de méthode status):
    //         // - on peut juste vérifier que le dossier existe et afficher le nb de fichiers
    //         // Mais l’idéal est d’ajouter runner.status() plus tard.
    //         std::cout << "[OK] migrations dir: " << dir << "\n";
    //         std::cout << "Tip: add a runner.status() API to show applied vs pending.\n";
    //         return 0;
    //     }

    //     usage(argv[0]);
    //     return 1;
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "[ERR] " << e.what() << "\n";
    //     return 1;
    // }
}
