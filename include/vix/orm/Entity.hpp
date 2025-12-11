#ifndef VIX_ENTITY_HPP
#define VIX_ENTITY_HPP

#include <cstdint>
#include <string>

namespace vix::orm
{
    struct Entity
    {
        virtual ~Entity() = default;
    };
} // namespace Vix::orm

#endif // VIX_ENTITY_HPP
