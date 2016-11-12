#pragma once

#include <gameui/uithemer.hpp>

#include <framework/modulehandler.hpp>

#include <reflection/magic.hpp>

#include "../RenderingKit.hpp"

namespace RenderingKit
{
    class IRKUIThemer : public gameui::UIThemer
    {
        public:
            virtual ~IRKUIThemer() {}

            virtual void Init(zfw::ISystem* sys, IRenderingKit* rk, zfw::IResourceManager* resRef) = 0;

            virtual IFontFace* GetFont(intptr_t fontIndex) = 0;

            REFL_CLASS_NAME("IRKUIThemer", 1)
    };

#ifdef RENDERING_KIT_STATIC
    IRKUIThemer* CreateRKUIThemer();

    ZOMBIE_IFACTORYLOCAL(RKUIThemer)
#else
    ZOMBIE_IFACTORY(RKUIThemer, "RenderingKit")
#endif
}
