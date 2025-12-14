#pragma once

#include <string>
#include <vector>

namespace vix::orm::tools
{
    class MigratorCLI
    {
    public:
        struct Options
        {
            std::string host;
            std::string user;
            std::string pass;
            std::string db;

            std::string command; // migrate | rollback | status
            std::string migrationsDir = "migrations";

            int steps = 0; // rollback steps
            bool help = false;
        };

        static int run(int argc, char **argv);

    private:
        static void printUsage(const char *prog);

        static Options parseArgsOrThrow(int argc, char **argv);

        static std::string getFlagValue(const std::vector<std::string> &args,
                                        const std::string &key);

        static bool hasFlag(const std::vector<std::string> &args,
                            const std::string &key);

        static std::string parseDir(const std::vector<std::string> &args);

        static int parseStepsOrThrow(const std::vector<std::string> &args);

        static void validateOrThrow(const Options &opt);
    };
} // namespace vix::orm::tools
