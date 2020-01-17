#ifndef LANTHAIA_TITLESCENE_HPP
#define LANTHAIA_TITLESCENE_HPP

#include <framework/scene.hpp>

#include <PubSub.hpp>

namespace Client {

class TitleScene : public zfw::IScene {
public:
    TitleScene(zfw::IEngine& engine, zfw::MessageQueue& eventQueue, PubSub::Broker& broker);

    bool Init() override;
    void Shutdown() override {}

    bool AcquireResources() override { return true; }
    void DropResources() override {}

    void OnTicks(int ticks) override;

private:
    zfw::IEngine& engine;
    zfw::MessageQueue& eventQueue;

    PubSub::Pipe myPipe;
    PubSub::Subscription sub;
};

}

#endif
