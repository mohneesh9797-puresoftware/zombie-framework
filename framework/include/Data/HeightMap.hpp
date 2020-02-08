#ifndef OBS_DATA_HEIGHTMAP_HPP
#define OBS_DATA_HEIGHTMAP_HPP

#include "Matrix2D.hpp"

#include <framework/pixmap.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace Obs::Data {

class HeightMap {
public:
    HeightMap(zfw::Pixmap_t const& pm);

    glm::ivec2 GetResolution() const { return heights.size; }

    // Bilinear-filtered sampling
    // Note that UV is always 0..1, regardless of heightmap resolution
    float SampleBilinear(glm::vec2 uv) const;

    void BuildMesh(glm::vec3 dimensions, glm::vec3 origin, glm::tvec2<unsigned> resolution, glm::vec2 uv0,
            glm::vec2 uv1, bool withNormals, bool withUvs, bool wireframe, std::vector<glm::vec3>& coords_out,
            std::vector<glm::vec3>& normals_out, std::vector<glm::vec2>& uvs_out, std::vector<int>& indices_out) const;

private:
    Matrix2D<float> heights;
};

}

#endif
