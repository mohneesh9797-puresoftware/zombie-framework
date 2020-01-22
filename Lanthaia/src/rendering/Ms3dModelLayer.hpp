#ifndef LANTHAIA_MS3DMODELLAYER_HPP
#define LANTHAIA_MS3DMODELLAYER_HPP

#include <Gfx/RenderingSystem.hpp>

#include "Ms3dModel.hpp"

namespace Client {

using namespace Obs::Gfx;

class Ms3dModelLayer: public zfw::IBroadcastSubscriber {
public:
    Ms3dModelLayer(zfw::IBroadcastHandler& bh, zfw::IEngine& engine, zfw::IEntityWorld2& world, zfw::IResourceManager2& resMgr,
            RenderingKit::IRenderingManager& rm);
    ~Ms3dModelLayer();

    void Render(RenderingContext const& ctx);

    // zfw::IBroadcastSubscriber
    void OnComponentEvent(zfw::IEntityWorld2& world, zfw::EntityId entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) override;
    void OnMessageBroadcast(intptr_t type, const void* payload) override {}

private:
    zfw::EntityViewWithCustomData<Ms3dModel*, zfw::Model3D, zfw::Position> renderables;
    void UpdateInPlace(zfw::IEntityWorld2& world, zfw::EntityId entityId, decltype(renderables)::References components, Ms3dModel*& model);

    Ms3dModelResourceProvider ms3dModelResourceProvider;

    zfw::IEntityWorld2& world;
    zfw::IResourceManager2& resMgr;
};

}

#endif
