#ifndef LANTHAIA_RENDERINGMANAGER_HPP
#define LANTHAIA_RENDERINGMANAGER_HPP

#include <RenderingKit/RenderingKit.hpp>

namespace Client {

class RenderingManager {
public:
    bool Startup(zfw::IEngine* sys, zfw::ErrorBuffer_t* eb, zfw::MessageQueue* eventQueue);

private:
    std::unique_ptr<RenderingKit::IRenderingKit> rk;
    RenderingKit::IRenderingManager* rm;
};

}

#endif
