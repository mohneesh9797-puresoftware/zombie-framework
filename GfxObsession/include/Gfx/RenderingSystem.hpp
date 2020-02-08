#ifndef OBS_GFX_RENDERINGSYSTEM_HPP
#define OBS_GFX_RENDERINGSYSTEM_HPP

#include "Fwd.hpp"

#include <RenderingKit/RenderingKit.hpp>
#include <RenderingKit/utility/BasicPainter.hpp>

#include <framework/broadcasthandler.hpp>
#include <framework/entityworld2.hpp>
#include <framework/utility/entityview.hpp>

#include <vector>

namespace Obs::Gfx {

class IRenderingSystem {
public:
    virtual void AddCustomLayer(std::function<void(RenderingContext const& ctx)> layerFunction, const char* name) = 0;
    virtual void AddEntityWorldLayer(zfw::IEntityWorld2& world, const char* name) = 0;
};

struct RenderLayer {
    // TODO: a variant would be more fitting
//    ICustomRenderLayer* custom = nullptr;
//    zfw::IEntityWorld2* world = nullptr;

    std::function<void(RenderingContext const& ctx)> function;

    std::string name;
};

class RenderingSystem : public IRenderingSystem, public zfw::IBroadcastSubscriber {
public:
    RenderingSystem(zfw::IResourceManager2& resMgr) : resMgr(resMgr) {}

    bool Startup(zfw::IEngine* sys, zfw::MessageQueue* eventQueue);

    // TODO: instead terrain handler
    // TODO: instead camera followed entity or someshit
    void DrawWorld(std::optional<zfw::EntityHandle> playerEntity);

    void OnComponentEvent(zfw::IEntityWorld2& world, zfw::EntityId entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) override;
    void OnMessageBroadcast(intptr_t type, const void* payload) override {}

    // TODO API: there should be no need for anyone to access my implementation details
    RenderingKit::IRenderingManager& GetRm() { return *rm; }

    // IRenderingSystem
//    void AddCustomLayer(ICustomRenderLayer& layer, const char* name) override;
    void AddCustomLayer(std::function<void(RenderingContext const& ctx)> layerFunction, const char* name) override;
    void AddEntityWorldLayer(zfw::IEntityWorld2& world, const char* name) override;

private:
    std::unique_ptr<RenderingKit::IRenderingKit> rk;
    RenderingKit::IRenderingManager* rm;

    // TODO API: instances shouldn't be necessary
    RenderingKit::BasicPainter3D<> bp3d;
    RenderingKit::BasicPainter2D<> bp2d;

    std::shared_ptr<RenderingKit::IFontFace> font;

    std::vector<RenderLayer> layers;

    zfw::IResourceManager2& resMgr;
};

}

#endif
