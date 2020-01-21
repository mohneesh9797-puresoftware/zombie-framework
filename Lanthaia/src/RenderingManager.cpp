#include "RenderingManager.hpp"

#include <framework/colorconstants.hpp>
#include <framework/engine.hpp>
#include <framework/errorcheck.hpp>
#include <framework/utility/essentials.hpp>

#include <RenderingKit/utility/Camera.hpp>
#include <RenderingKit/utility/RKVideoHandler.hpp>

using namespace RenderingKit;
using std::make_unique;

namespace Client {

bool RenderingManager::Startup(zfw::IEngine* sys, zfw::MessageQueue* eventQueue) {
    auto imh = sys->GetModuleHandler(true);

    this->rk.reset(RenderingKit::TryCreateRenderingKit(imh));
    zombie_ErrorCheck(rk);

    if (!rk->Init(sys)) {
        return false;
    }

    auto wm = rk->GetWindowManager();

    // TODO API: refactor stateful LoadDefaultSettings
    // TODO API: consider enabling anti-alias by default
    if (!wm->LoadDefaultSettings(nullptr) || !wm->ResetVideoOutput()) {
        return false;
    }

    this->rm = rk->StartupRendering(CoordinateSystem::rightHanded);
    zombie_ErrorCheck(rm);

    sys->SetVideoHandler(make_unique<RKVideoHandler>(rm, wm, eventQueue));

    zombie_ErrorCheck(bp2d.Init(rm));
    zombie_ErrorCheck(bp3d.Init(rm));

    return true;
}

void RenderingManager::DrawWorld() {
    rm->Clear(zfw::COLOUR_WHITE);

    // TODO API: it's too easy to forget something
    Camera cam(CoordinateSystem::leftHanded);
    cam.SetClippingDist(1.0f, 1000.0f);
    cam.SetVFov(3.14f * 0.25f);
    cam.SetPerspective();
    cam.SetViewWithCenterDistanceYawPitch({}, 20.0f, 0.0f, 3.14f*0.25f);
    rm->SetCamera(&cam);

    bp3d.DrawGridAround({}, {1.0f, 1.0f, 0.0f}, {20, 20}, zfw::RGBA_BLACK);
}

}
