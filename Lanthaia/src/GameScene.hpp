#ifndef LANTHAIA_GAMESCENE_HPP
#define LANTHAIA_GAMESCENE_HPP

#include <framework/scene.hpp>

namespace Client {

class RenderingSystem;

class GameScene : public zfw::IScene {
public:
    GameScene(RenderingSystem& r)
        : r(r) {}

    bool Init() override { return true; }
    void Shutdown() override {}

    bool AcquireResources() override { return true; }
    void DropResources() override {}

    void DrawScene() override;

private:
    RenderingSystem& r;
};

}

#endif
