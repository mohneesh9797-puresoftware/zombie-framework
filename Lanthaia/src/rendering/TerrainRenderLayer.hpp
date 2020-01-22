#ifndef LANTHAIA_TERRAINRENDERLAYER_HPP
#define LANTHAIA_TERRAINRENDERLAYER_HPP

#include <Ecs/TerrainHeightMap.hpp>
#include <Gfx/RenderingSystem.hpp>

namespace Client {

using namespace Obs::Gfx;
using Obs::Ecs::TerrainHeightMap;

class TerrainRenderLayer: public zfw::IBroadcastSubscriber {
public:
    TerrainRenderLayer(zfw::IBroadcastHandler& bh, zfw::IEngine& engine, zfw::IEntityWorld2& world, zfw::IResourceManager2& resMgr);

    void Render(RenderingContext const& ctx);

    // zfw::IBroadcastSubscriber
    void OnComponentEvent(zfw::IEntityWorld2& world, zfw::EntityId entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) override;
    void OnMessageBroadcast(intptr_t type, const void* payload) override {}

private:
    struct RenderableWorldMesh {
        RenderingKit::IMaterial* material = nullptr;
        std::shared_ptr<RenderingKit::IGeomBuffer> gb;
        std::unique_ptr<RenderingKit::IGeomChunk> gc;
    };

    zfw::EntityViewWithCustomData<RenderableWorldMesh, TerrainHeightMap, zfw::Position> renderables;
    void UpdateInPlace(zfw::IEntityWorld2& world, zfw::EntityId entityId, decltype(renderables)::References components, RenderableWorldMesh& mesh);

    zfw::IEngine& engine;
    zfw::IEntityWorld2& world;
    zfw::IResourceManager2& resMgr;
};

}

#endif
