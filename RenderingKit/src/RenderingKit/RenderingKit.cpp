
#include "RenderingKitImpl.hpp"

#include <framework/shader_preprocessor.hpp>
#include <framework/engine.hpp>

namespace RenderingKit
{
    using namespace zfw;

    IRenderingKit* CreateRenderingKit()
    {
        return new RenderingKit();
    }

    RenderingKit::RenderingKit()
    {
        sys = nullptr;
        eb = nullptr;
    }

    zfw::ShaderPreprocessor* RenderingKit::GetShaderPreprocessor()
    {
        if (shaderPreprocessor == nullptr)
            shaderPreprocessor.reset(sys->CreateShaderPreprocessor());

        return shaderPreprocessor.get();
    }

#if ZOMBIE_API_VERSION >= 202001
    bool RenderingKit::Init(IEngine* sys)
#else
    bool RenderingKit::Init(IEngine* sys, ErrorBuffer_t* eb, IRenderingKitHost* host)
#endif
    {
        SetEssentials(sys->GetEssentials());

        this->sys = sys;
        this->eb = sys->GetEssentials()->GetErrorBuffer();

        wm.reset(CreateSDLWindowManager(eb, this));

        if (!wm->Init())
            return false;

        return true;
    }

    IRenderingManager* RenderingKit::StartupRendering(CoordinateSystem coordSystem)
    {
        rm = IRenderingManagerBackend::Create(eb, this, coordSystem);

        if (!rm->Startup()) {
            return nullptr;
        }

        return rm.get();
    }
}
