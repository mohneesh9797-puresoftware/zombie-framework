#include "RenderingManager.hpp"

#include <framework/engine.hpp>
#include <framework/errorcheck.hpp>
#include <RenderingKit/utility/RKVideoHandler.hpp>

using namespace RenderingKit;
using std::make_unique;

namespace Client {

bool RenderingManager::Startup(zfw::IEngine* sys, zfw::ErrorBuffer_t* eb, zfw::MessageQueue* eventQueue) {
    auto imh = sys->GetModuleHandler(true);

    this->rk.reset(RenderingKit::TryCreateRenderingKit(imh));
    zombie_ErrorCheck(rk);

    if (!rk->Init(sys)) {
        return false;
    }

    auto wm = rk->GetWindowManager();

    if (!wm->LoadDefaultSettings(nullptr) || !wm->ResetVideoOutput()) {
        return false;
    }

    this->rm = rk->StartupRendering(CoordinateSystem::rightHanded);
    zombie_ErrorCheck(rm);

    sys->SetVideoHandler(make_unique<RKVideoHandler>(rm, wm, eventQueue));

    return true;
}

}
