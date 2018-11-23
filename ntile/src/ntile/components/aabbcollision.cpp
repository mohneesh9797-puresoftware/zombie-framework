#include "aabbcollision.hpp"

#include <framework/componenttype.hpp>

namespace zfw
{
    static BasicComponentType<AabbCollision> type;

    template <>
    IComponentType& GetComponentType<AabbCollision>() {
        return type;
    }
}
