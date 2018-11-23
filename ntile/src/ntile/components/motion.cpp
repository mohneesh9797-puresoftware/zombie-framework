#include "motion.hpp"

#include <framework/componenttype.hpp>

namespace zfw
{
    static BasicComponentType<Motion> type;

    template <>
    IComponentType& GetComponentType<Motion>() {
        return type;
    }
}
