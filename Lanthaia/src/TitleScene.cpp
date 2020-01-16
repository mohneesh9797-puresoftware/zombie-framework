#include "TitleScene.hpp"

#include <framework/engine.hpp>
#include <framework/event.hpp>
#include <framework/messagequeue.hpp>

namespace Client {

void TitleScene::OnTicks(int ticks) {
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
