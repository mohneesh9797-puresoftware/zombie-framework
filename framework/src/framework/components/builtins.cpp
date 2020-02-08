#include <Ecs/TerrainHeightMap.hpp>
#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/componenttype.hpp>

namespace zfw
{
    template <>
    IComponentType& GetComponentType<Model3D>() {
        static BasicComponentType<Model3D> type;

        return type;
    }

    template <>
    IComponentType& GetComponentType<Position>() {
        static BasicComponentType<Position> type;

        return type;
    }

    template <>
    IComponentType& GetComponentType<Obs::Ecs::TerrainHeightMap>() {
        static BasicComponentType<Obs::Ecs::TerrainHeightMap> type;

        return type;
    }
}
