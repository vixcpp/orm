#include <vix/orm/FileMigrationsRunner.hpp>
#include <vix/orm/Errors.hpp>
#include <vix/orm/Sha256.hpp>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace vix::orm
{
    static std::string now_text()
    {
        // keep DB-agnostic: store a simple marker
        // you can replace with CURRENT_TIMESTAMP via SQL if you prefer
        return "now";
    }

    void FileMigrationsRunner::ensureTable()
    {
        // DB-agnostic table:
        // id: primary key, checksum: sha256(up), applied_at: text
        const std::string sql =
            "CREATE TABLE IF NOT EXISTS " + table_ + " ("
                                                     "  id VARCHAR(255) NOT NULL PRIMARY KEY,"
                                                     "  checksum VARCHAR(64) NOT NULL,"
                                                     "  applied_at VARCHAR(64) NOT NULL"
                                                     ");";
        conn_.prepare(sql)->exec();
    }

    std::string FileMigrationsRunner::readFileText(const std::filesystem::path &p)
    {
        std::ifstream in(p);
        if (!in)
            throw DBError("Cannot open migration file: " + p.string());
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    std::string FileMigrationsRunner::trim(std::string s)
    {
        auto is_ws = [](unsigned char c)
        { return std::isspace(c); };
        while (!s.empty() && is_ws(static_cast<unsigned char>(s.front())))
            s.erase(s.begin());

        while (!s.empty() && is_ws(static_cast<unsigned char>(s.back())))
            s.pop_back();

        return s;
    }

    std::vector<std::string> FileMigrationsRunner::splitStatements(const std::string &sql)
    {
        std::vector<std::string> out;
        std::string cur;
        cur.reserve(sql.size());

        bool in_sq = false, in_dq = false;

        for (size_t i = 0; i < sql.size(); ++i)
        {
            char c = sql[i];
            if (c == '\'' && !in_dq)
                in_sq = !in_sq;
            if (c == '"' && !in_sq)
                in_dq = !in_dq;

            if (c == ';' && !in_sq && !in_dq)
            {
                auto stmt = trim(cur);
                if (!stmt.empty())
                    out.push_back(std::move(stmt));
                cur.clear();
            }
            else
            {
                cur.push_back(c);
            }
        }

        auto last = trim(cur);
        if (!last.empty())
            out.push_back(std::move(last));
        return out;
    }

    void FileMigrationsRunner::execScript(const std::string &sql)
    {
        auto stmts = splitStatements(sql);
        for (auto &s : stmts)
            conn_.prepare(s)->exec();
    }

    // ---------------- scan pairs ----------------

    static bool ends_with(const std::string &s, const std::string &suf)
    {
        return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
    }

    static std::string strip_suffix(const std::string &s, const std::string &suf)
    {
        return s.substr(0, s.size() - suf.size());
    }

    std::vector<MigrationPair> FileMigrationsRunner::scanPairs() const
    {
        if (!std::filesystem::exists(dir_))
            throw DBError("Migrations directory does not exist: " + dir_.string());

        // Collect by id
        struct Tmp
        {
            std::filesystem::path up, down;
        };
        std::unordered_map<std::string, Tmp> map;

        for (auto const &entry : std::filesystem::directory_iterator(dir_))
        {
            if (!entry.is_regular_file())
                continue;
            const auto name = entry.path().filename().string();

            if (ends_with(name, ".up.sql"))
            {
                const auto id = strip_suffix(name, ".up.sql");
                map[id].up = entry.path();
            }
            else if (ends_with(name, ".down.sql"))
            {
                const auto id = strip_suffix(name, ".down.sql");
                map[id].down = entry.path();
            }
        }

        std::vector<MigrationPair> out;
        out.reserve(map.size());

        for (auto &[id, t] : map)
        {
            if (t.up.empty())
                continue; // ignore orphan down

            MigrationPair mp;
            mp.id = id;
            mp.up_path = t.up;
            mp.down_path = t.down;

            // checksum of the UP content
            auto up_sql = readFileText(mp.up_path);
            mp.up_checksum = sha256_hex(up_sql);

            out.push_back(std::move(mp));
        }

        std::sort(out.begin(), out.end(), [](const auto &a, const auto &b)
                  {
                      return a.id < b.id; // timestamp prefix => correct order
                  });

        return out;
    }

    // ---------------- db ops ----------------

    bool FileMigrationsRunner::isApplied(const std::string &id, std::string *checksum_out)
    {
        auto st = conn_.prepare("SELECT checksum FROM " + table_ + " WHERE id = ?");
        st->bind(1, id);
        auto rs = st->query();

        if (!rs->next())
            return false;

        if (checksum_out)
            *checksum_out = rs->row()->getString(0); // col 0 = checksum

        return true;
    }

    void FileMigrationsRunner::markApplied(const std::string &id, const std::string &checksum)
    {
        auto st = conn_.prepare("INSERT INTO " + table_ + " (id, checksum, applied_at) VALUES (?, ?, ?)");
        st->bind(1, id);
        st->bind(2, checksum);
        st->bind(3, now_text());
        st->exec();
    }

    void FileMigrationsRunner::unmarkApplied(const std::string &id)
    {
        auto st = conn_.prepare("DELETE FROM " + table_ + " WHERE id = ?");
        st->bind(1, id);
        st->exec();
    }

    std::string FileMigrationsRunner::lastAppliedId()
    {
        // Works because id is timestamp-prefixed
        auto st = conn_.prepare("SELECT id FROM " + table_ + " ORDER BY id DESC LIMIT 1");
        auto rs = st->query();
        if (!rs->next())
            return {};
        return rs->row()->getString(0);
    }

    // ---------------- public API ----------------

    void FileMigrationsRunner::applyAll()
    {
        ensureTable();
        auto pairs = scanPairs();

        for (const auto &m : pairs)
        {
            std::string existing_checksum;
            if (isApplied(m.id, &existing_checksum))
            {
                // Detect modified file after apply
                if (existing_checksum != m.up_checksum)
                {
                    throw DBError(
                        "Migration already applied but checksum changed: " + m.id +
                        "\n  db:  " + existing_checksum +
                        "\n  file:" + m.up_checksum);
                }
                continue;
            }

            try
            {
                conn_.begin();
                execScript(readFileText(m.up_path));
                markApplied(m.id, m.up_checksum);
                conn_.commit();
            }
            catch (...)
            {
                try
                {
                    conn_.rollback();
                }
                catch (...)
                {
                }
                throw;
            }
        }
    }

    void FileMigrationsRunner::rollback(int steps)
    {
        ensureTable();
        if (steps <= 0)
            return;

        // For rollback we need the down file; we scan once to map id -> pair
        auto pairs = scanPairs();
        std::unordered_map<std::string, MigrationPair> byId;
        byId.reserve(pairs.size());
        for (auto &p : pairs)
            byId.emplace(p.id, p);

        for (int i = 0; i < steps; ++i)
        {
            const auto id = lastAppliedId();
            if (id.empty())
                throw DBError("No applied migrations to rollback.");

            auto it = byId.find(id);
            if (it == byId.end())
                throw DBError("Cannot rollback: migration files not found for id: " + id);

            const auto &m = it->second;
            if (m.down_path.empty())
                throw DBError("Cannot rollback: missing .down.sql for migration: " + id);

            try
            {
                conn_.begin();
                execScript(readFileText(m.down_path));
                unmarkApplied(id);
                conn_.commit();
            }
            catch (...)
            {
                try
                {
                    conn_.rollback();
                }
                catch (...)
                {
                }
                throw;
            }
        }
    }
}
