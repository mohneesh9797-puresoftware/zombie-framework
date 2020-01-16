#ifndef LANTHAIA_TITLESCENE_HPP
#define LANTHAIA_TITLESCENE_HPP

#include <framework/scene.hpp>

namespace Client {

class TitleScene : public zfw::IScene {
public:
    TitleScene(zfw::IEngine& engine, zfw::MessageQueue& eventQueue) : engine(engine), eventQueue(eventQueue) {}

    bool Init() override { return true; }
    void Shutdown() override {}

    bool AcquireResources() override { return true; }
    void DropResources() override {}

    void OnTicks(int ticks) override;

private:
    zfw::IEngine& engine;
    zfw::MessageQueue& eventQueue;
};

}

#endif
