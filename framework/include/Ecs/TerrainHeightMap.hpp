#ifndef OBS_ECS_TERRAINHEIGHTMAP_HPP
#define OBS_ECS_TERRAINHEIGHTMAP_HPP

#include <Base/u8string.hpp>
#include <Data/HeightMap.hpp>

#include <glm/vec3.hpp>

namespace Obs::Ecs {

struct TerrainHeightMap {
    u8string path;
    u8string texture;

    glm::vec3 scale;
};

}

#endif
