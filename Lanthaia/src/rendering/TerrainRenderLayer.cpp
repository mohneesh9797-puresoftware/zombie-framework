#include "TerrainRenderLayer.hpp"

#include <Gfx/RenderingContext.hpp>
#include <Res/ResourceHeightMap.hpp>

#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/componenttype.hpp>
#include <framework/engine.hpp>
#include <framework/resourcemanager2.hpp>

#include <glm/gtc/quaternion.hpp>

using glm::mat4x4;
using glm::vec2;
using glm::vec3;
using std::get;

namespace Client {

struct TerrainVertex_t {
    float pos[3];
    float normal[3];
    float uv[2];
    float rgba[4]; // TODO find out and take note of how to avoid this if shader requires, but we don't care
};

TerrainRenderLayer::TerrainRenderLayer(zfw::IBroadcastHandler& bh, zfw::IEngine& engine, zfw::IEntityWorld2& world, zfw::IResourceManager2& resMgr)
        : renderables([this](auto&& ...args) { UpdateInPlace(args...); }), engine(engine), world(world), resMgr(resMgr) {
    renderables.SubscribeToComponentTypes(bh, *this);
}

void TerrainRenderLayer::OnComponentEvent(zfw::IEntityWorld2& world, intptr_t entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) {
    if (&world != &this->world) {
        return;
    }

    renderables.OnComponentEvent(world, entityId, type, data, event);
}

void TerrainRenderLayer::Render(RenderingContext const& ctx) {
    auto& rm = ctx.rm;
    
    for (auto& [entityId, renderable] : renderables.entities) {
        auto& [rwm, components] = renderable;

        auto& terrain = get<TerrainHeightMap&>(components);

        if (!rwm.gc) {
            // build geometry (vertices)

            std::vector<vec3> coords, normals;
            std::vector<vec2> uvs;
            std::vector<int> indices;

            auto heightMapRes = resMgr.GetResourceByPath<Obs::Res::ResourceHeightMap>(terrain.path, zfw::IResourceManager2::kResourceRequired);
            zombie_assert(heightMapRes);

            auto& heightMap = heightMapRes->Get();

            heightMap.BuildMesh(terrain.scale, {}, heightMap.GetResolution(),
                    {}, { terrain.scale.x * 0.2f, terrain.scale.y * 0.2f }, true, true, false, coords, normals, uvs, indices);

            // allocate a geomchunk, convert vertices etc.
            char* materialRecipe = zfw::Params::BuildAlloc(2, "shader", "path=RenderingKit/basicTextured", "texture:tex",
                                                           ("path=" + terrain.texture + ",wrapx=repeat,wrapy=repeat").c_str());

            rwm.material = resMgr.GetResource<RenderingKit::IMaterial>(materialRecipe, zfw::IResourceManager2::kResourceRequired);
            free(materialRecipe);

            if (!rwm.material) {
                engine.PrintError(nullptr, zfw::kLogError);
                zombie_assert(rwm.material);
            }

            std::vector<TerrainVertex_t> vertices;

            for (size_t i = 0; i < indices.size(); i++) {
                int idx = indices[i];
                vertices.emplace_back(TerrainVertex_t { { coords[idx].x, coords[idx].y, coords[idx].z },
                                                        { normals[idx].x, normals[idx].y, normals[idx].z }, { uvs[idx].x, uvs[idx].y },
                                                        { 1.0f, 1.0f, 1.0f, 1.0f } });
            }

            static const RenderingKit::VertexAttrib_t worldVertexAttribs[] = {
                    { "in_Position", 0, RenderingKit::RK_ATTRIB_FLOAT_3 },
                    { "in_Normal", 12, RenderingKit::RK_ATTRIB_FLOAT_3 },
                    { "in_UV", 24, RenderingKit::RK_ATTRIB_FLOAT_2 },
                    { "in_Color", 32, RenderingKit::RK_ATTRIB_FLOAT_4 },
            };
            static void* vfiCache = nullptr;
            static const RenderingKit::VertexFormatInfo vfi { /*.vertexSizeInBytes =*/sizeof(TerrainVertex_t),
                    /*.attribs =*/worldVertexAttribs,
                    /*.numAttribs =*/std::size(worldVertexAttribs),
                    /*.cache =*/&vfiCache };
            
            // auto vf = rm.CompileVertexFormat(material->GetShader(), sizeof(TerrainVertex_t),
            // worldVertexAttribs, false);
            rwm.gb = rm.CreateGeomBuffer("heightmap");
            rwm.gc = rwm.gb->AllocVertices(vfi, vertices.size(), 0);
            rwm.gc->UpdateVertices(0, &vertices[0], vertices.size() * sizeof(TerrainVertex_t));
        }

        auto& position = get<zfw::Position&>(components);

        rm.DrawPrimitivesTransformed(rwm.material, RenderingKit::RK_TRIANGLES, rwm.gc.get(),
                                     glm::translate(mat4x4(1.0f), position.pos));
    }
}

void TerrainRenderLayer::UpdateInPlace(zfw::IEntityWorld2& world, zfw::EntityId entityId, decltype(renderables)::References components, RenderableWorldMesh& mesh) {
}

}