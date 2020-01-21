#ifndef LANTHAIA_RENDERINGMANAGER_HPP
#define LANTHAIA_RENDERINGMANAGER_HPP

#include <RenderingKit/RenderingKit.hpp>
#include <RenderingKit/utility/BasicPainter.hpp>

namespace Client {

class RenderingManager {
public:
    bool Startup(zfw::IEngine* sys, zfw::MessageQueue* eventQueue);

    void DrawWorld();

private:
    std::unique_ptr<RenderingKit::IRenderingKit> rk;
    RenderingKit::IRenderingManager* rm;

    // TODO API: instances shouldn't be necessary
    RenderingKit::BasicPainter3D<> bp3d;
    RenderingKit::BasicPainter2D<> bp2d;
};

}

#endif
