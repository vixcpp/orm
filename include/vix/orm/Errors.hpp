#ifndef VIX_ERRORS_HPP
#define VIX_ERRORS_HPP

#include <string>
#include <stdexcept>

namespace vix::orm
{
    struct DBError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct TxnError : DBError
    {
        using DBError::DBError;
    };

    struct NotFound : DBError
    {
        using DBError::DBError;
    };
} // namespace Vix::orm

#endif // VIX_ERRORS_HPP
