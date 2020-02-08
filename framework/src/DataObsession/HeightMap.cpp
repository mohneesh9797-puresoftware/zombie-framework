#include <Data/HeightMap.hpp>

#include <framework/utility/pixmap.hpp>

namespace Obs::Data {

using glm::vec2;
using glm::vec3;

static const unsigned offsets[2][3][2] = { { { 0, 0 }, { 0, 1 }, { 1, 0 } }, { { 1, 0 }, { 0, 1 }, { 1, 1 } } };

static float filter(float a) {
    return a < 0.5f ? 2 * a * a : 1.0f - 2 * (1.0f - a) * (1.0f - a);
}

HeightMap::HeightMap(zfw::Pixmap_t const& pm, float zscale) : heights(pm.info.size) {
    const float factor = (zscale / 255.0f);

    zombie_assert(pm.info.format == zfw::PixmapFormat_t::BGR8 ||
                  pm.info.format == zfw::PixmapFormat_t::RGB8);

    auto const stride = zfw::Pixmap::GetBytesPerLine(pm.info);

    for (int y = 0; y < pm.info.size.y; y++) {
        for (int x = 0; x < pm.info.size.x; x++) {
            auto z = pm.pixelData[y * stride + x * 3] * factor;
            heights.UnsafeSet(x, y, z);
        }
    }
}

void HeightMap::BuildMesh(vec3 dimensions, glm::vec3 origin, glm::tvec2<unsigned> resolution,
    vec2 uv0, vec2 uv1, bool withNormals, bool withUvs, bool wireframe, std::vector<vec3>& coords_out,
    std::vector<vec3>& normals_out, std::vector<vec2>& uvs_out,
    std::vector<int>& indices_out) const {
    vec2 maxSample(resolution.x - 1, resolution.y - 1);

    vec2 spacing(vec2(dimensions) / maxSample);
    vec2 uvSpacing((uv1 - uv0) / maxSample);

    for (unsigned y = 0; y < resolution.y; y++) {
        for (unsigned x = 0; x < resolution.x; x++) {
            auto z = SampleBilinear(glm::vec2(x / maxSample.x, y / maxSample.y)) * dimensions.z - origin.z;
            coords_out.push_back({ x * spacing.x - origin.x, y * spacing.y - origin.y, z });

            /*vertex.nX = 0.0f;
            vertex.nY = 0.0f;
            vertex.nZ = 0.0f;*/

            normals_out.push_back({});

            uvs_out.push_back({ uv0.x + x * uvSpacing.x, uv0.y + y * uvSpacing.y });

            if (x < resolution.x - 1 && y < resolution.y - 1) {
                if (!wireframe) {
                    indices_out.push_back(y * resolution.x + x + 1);
                    indices_out.push_back(y * resolution.x + x);
                    indices_out.push_back((y + 1) * resolution.x + x + 1);

                    indices_out.push_back((y + 1) * resolution.x + x + 1);
                    indices_out.push_back(y * resolution.x + x);
                    indices_out.push_back((y + 1) * resolution.x + x);
                }
                else {
                    indices_out.push_back(y * resolution.x + x + 1);
                    indices_out.push_back(y * resolution.x + x);

                    indices_out.push_back(y * resolution.x + x);
                    indices_out.push_back((y + 1) * resolution.x + x);

                    indices_out.push_back((y + 1) * resolution.x + x);
                    indices_out.push_back(y * resolution.x + x + 1);
                }
            }
        }
    }

    // Calculate normals
    vec3 corners[3], sides[2], normal;
    std::vector<unsigned> connections(coords_out.size());

    for (unsigned i = 0; i < indices_out.size(); i += 3) {
        // Get the corners of this polygon
        for (unsigned j = 0; j < 3; j++)
            corners[j] = coords_out[indices_out[i + j]];

        // Calculate vectors representing two sides of this poly
        sides[0] = corners[1] - corners[0];
        sides[1] = corners[2] - corners[0];

        // Get the cross-product of those two vectors
        //  and normalize it to get a direction vector.
        normal = glm::normalize(glm::cross(sides[0], sides[1]));

        for (unsigned j = 0; j < 3; j++) {
            // Remember that the normals for this polygon were calculated,
            //  accumulate this information per-vertex
            connections[indices_out[i + j]]++;

            // And add the normal itself to the normal-sum
            normals_out[indices_out[i + j]] += normal;
        }
    }

    // Loop through the vertices and divide every normal-sum by the calculation-count
    // for each_in_list ( vertices, i )
    for (size_t i = 0; i < coords_out.size(); i++) {
        if (connections[i] > 0) {
            normals_out[i] /= connections[i];
        }

        // In order for diffuse to work, we need all the normals to point up
        if (normals_out[i].z < 0) {
            normals_out[i] = -normals_out[i];
        }
    }

//    printf("createFromHeightMap: Generated %zu vertices, %zu indices\n", coords_out.size(), indices_out.size());
}

float HeightMap::SampleBilinear(glm::vec2 uv) const {
    uv = { uv.x * (heights.size.x - 1), uv.y * (heights.size.y - 1) };

    const glm::vec2 uv0 { floor(uv.x), floor(uv.y) }, uv1 { ceil(uv.x), ceil(uv.y) };

    constexpr float default_ = 0.0f;

    if (uv0 == uv1) {
        // Woohoo! Exact hit!

        return heights.Get((unsigned)uv0.x, (unsigned)uv0.y, default_);
    }
    else if (uv0.x == uv1.x) {
        // X is integer, linear-interpolate Y

        float uv0sample = heights.Get((unsigned)uv0.x, (unsigned)uv0.y, default_);
        float uv1sample = heights.Get((unsigned)uv1.x, (unsigned)uv1.y, default_);

        return uv0sample + (uv1sample - uv0sample) * filter(uv.y - uv0.y);
    }
    else if (uv0.y == uv1.y) {
        // Y is integer, linear-interpolate X

        float uv0sample = heights.Get((unsigned)uv0.x, (unsigned)uv0.y, default_);
        float uv1sample = heights.Get((unsigned)uv1.x, (unsigned)uv1.y, default_);

        return uv0sample + (uv1sample - uv0sample) * filter(uv.x - uv0.x);
    }
    else {
        // Ouch! Some nasty stuff has to happen here...

        return heights.Get((unsigned)uv0.x, (unsigned)uv0.y, default_) * filter(uv1.x - uv.x) * filter(uv1.y - uv.y) +
                heights.Get((unsigned)uv1.x, (unsigned)uv0.y, default_) * filter(uv.x - uv0.x) * filter(uv1.y - uv.y) +
                heights.Get((unsigned)uv0.x, (unsigned)uv1.y, default_) * filter(uv1.x - uv.x) * filter(uv.y - uv0.y) +
                heights.Get((unsigned)uv1.x, (unsigned)uv1.y, default_) * filter(uv.x - uv0.x) * filter(uv.y - uv0.y);
    }
}

}
