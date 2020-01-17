#include "system/LoginSystem.hpp"
#include "TitleScene.hpp"

#include <framework/engine.hpp>
#include <framework/event.hpp>
#include <framework/messagequeue.hpp>

namespace Client {

using std::make_unique;

TitleScene::TitleScene(zfw::IEngine& engine, zfw::MessageQueue& eventQueue, PubSub::Broker& broker)
        : engine(engine), eventQueue(eventQueue), sub(broker, myPipe) {
    sub.add<LoginServerInfo>();
    sub.add<LoginSession::StateUpdate>();
}

bool TitleScene::Init() {
    // TODO: init GUI
    // alt TODO: use stateless gui (from Container)

    engine.AddSystem(make_unique<LoginSystem>(sub.getBroker()));
    return true;
}

void TitleScene::OnTicks(int ticks) {
    while (auto msg = myPipe.poll()) {
        if (auto info = msg->cast<LoginServerInfo>()) {
            engine.Printf(zfw::kLogDebugInfo, "Realm name: %s", info->realmName.c_str());
            engine.Printf(zfw::kLogDebugInfo, "Realm MOTD: %s", info->realmNews.c_str());
        }
        else if (auto update = msg->cast<LoginSession::StateUpdate>()) {
            if (update->state == LoginSession::State::readyToLogin) {
                sub.getBroker().publish<LoginRequest>("minexew", "password");
            }
        }
    }

    while (zfw::MessageHeader* msg = eventQueue.Retrieve(li::Timeout(0))) {
        switch (msg->type) {
            case zfw::EVENT_WINDOW_CLOSE:
                engine.StopMainLoop();
                break;
        }

        msg->Release();
    }
}

}
