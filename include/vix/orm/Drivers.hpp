#ifndef VIX_ORM_DRIVERS_HPP
#define VIX_ORM_DRIVERS_HPP

#include <any>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>

namespace vix::orm
{
    struct ResultRow
    {
        virtual ~ResultRow() = default;
        virtual bool isNull(std::size_t i) const = 0;
        virtual std::string getString(std::size_t i) const = 0;
        virtual std::int64_t getInt64(std::size_t i) const = 0;
        virtual double getDouble(std::size_t i) const = 0;
    };

    struct ResultSet
    {
        virtual ~ResultSet() = default;
        virtual bool next() = 0;
        virtual std::size_t cols() const = 0;
        virtual std::unique_ptr<ResultRow> row() const = 0;
    };

    struct Statement
    {
        virtual ~Statement() = default;
        virtual void bind(std::size_t idx, const std::any &v) = 0;
        virtual std::unique_ptr<ResultSet> query() = 0;
        virtual std::uint64_t exec() = 0;
    };

    struct Connection
    {
        virtual ~Connection() = default;
        virtual std::unique_ptr<Statement> prepare(std::string_view sql) = 0;
        virtual void begin() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;
        virtual std::uint64_t lastInsertId() = 0;
    };

    using ConnectionPtr = std::shared_ptr<Connection>;
    using ConnectionFactory = std::function<ConnectionPtr()>;

    ConnectionFactory make_mysql_factory(std::string host,
                                         std::string user,
                                         std::string pass,
                                         std::string db);

} // namespace vix::orm

#endif // VIX_ORM_DRIVERS_HPP
