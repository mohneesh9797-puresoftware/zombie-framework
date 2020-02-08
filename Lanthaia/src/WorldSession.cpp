
#include "WorldSession.hpp"
#include "../Common/Messages.hpp"

static bool lost = false;

namespace Client {

using std::move;
using std::unique_ptr;

WorldSession::WorldSession(PubSub::Broker& broker, unique_ptr<li::TcpSocket> socket)
    : sub(broker, myPipe)
    , socket(move(socket)) {
}

void WorldSession::Chat(const std::string& text) {
    if (!socket)
        return;

    buffer.clear();
    buffer.writeLE<uint16_t>(world::say);
    buffer.writeString(text.c_str());
    socket->send(buffer);
}

void WorldSession::movement(const zfw::Float3& pos, float angle) {
    if (!socket)
        return;

    buffer.clear();
    buffer.writeLE<uint16_t>(world::player_movement);
    buffer.writeLE<float>(pos.x);
    buffer.writeLE<float>(pos.y);
    buffer.writeLE<float>(pos.z);
    buffer.writeLE<float>(angle);
    socket->send(buffer);
}

void WorldSession::Update() {
    if (!socket)
        return;

    if (socket->eof()) {
        if (!lost) {
            // TODO: forward errno/strerror
            printf("WorldSession: connection lost\n");
            sub.getBroker().publish<RealmSessionLost>();
        }

        lost = true;

        // socket->release();
        // socket = 0;

        return;
    }

    if (socket->receive(buffer)) {
        buffer.dump();

        uint16_t messageID;
        buffer.readLE(&messageID);

        switch (messageID) {
        case world::welcome: {
            uint16_t pid;
            uint16_t race, classID, level, zoneID;
            float x, y, z, orientation;
            uint32_t goldAmount;

            buffer.readLE(&pid);
            this->playerPid = pid;
            li::String name = buffer.readString();
            li::String location = buffer.readString();
            buffer.readLE(&race);
            buffer.readLE(&classID);
            buffer.readLE(&level);
            buffer.readLE(&zoneID);
            buffer.readLE(&x);
            buffer.readLE(&y);
            buffer.readLE(&z);
            buffer.readLE(&orientation);
            buffer.readLE(&goldAmount);

            printf("Server Hello: %s, %s, %u, [%g %g %g %g], %u\n", name.c_str(), location.c_str(), zoneID, x, y, z,
                orientation, goldAmount);

            sub.getBroker().publish<RealmMyCharacterInfo>(RealmMyCharacterInfo {
                /*.pid =*/pid,
                /*.name =*/name.c_str(),
                /*.pos =*/ { x, y, z },
                /*.orientation =*/orientation,
            });
            break;
        }

        case world::chat_message: {
            uint16_t channel;
            buffer.readLE(&channel);
            li::String from = buffer.readString();
            li::String message = buffer.readString();

            sub.getBroker().publish<RealmChatMessage>(from.c_str(), message.c_str());
            break;
        }

        case world::player_left_area: {
            uint16_t pid;
            buffer.readLE(&pid);
            sub.getBroker().publish<RealmEntityRemoval>(pid);
            break;
        }

        case world::player_list: {
            uint16_t playersNear;
            buffer.readLE(&playersNear);

            for (unsigned i = 0; i < playersNear; i++) {
                uint16_t pid;
                uint16_t race, classID, level;
                float x, y, z, orientation;

                buffer.readLE(&pid);
                li::String name = buffer.readString();
                buffer.readLE(&race);
                buffer.readLE(&classID);
                buffer.readLE(&level);
                buffer.readLE(&x);
                buffer.readLE(&y);
                buffer.readLE(&z);
                buffer.readLE(&orientation);

                sub.getBroker().publish<RealmEntityAddition>(RealmEntityAddition {
                    /*.pid =*/pid,
                    /*.name =*/name.c_str(),
                    /*.pos =*/ { x, y, z },
                    /*.orientation =*/orientation,
                });
            }
            break;
        }

        case world::player_location: {
            uint16_t pid;
            float x, y, z, orientation;

            buffer.readLE(&pid);
            buffer.readLE(&x);
            buffer.readLE(&y);
            buffer.readLE(&z);
            buffer.readLE(&orientation);

            if (playerPid == pid)
                printf("!!! WTF !!! received movement notify for OWN character! fix that Asap!!\n");

            sub.getBroker().publish<RealmEntityPositionUpdate>(RealmEntityPositionUpdate {
                /*.pid =*/pid,
                /*.pos =*/ { x, y, z },
                /*.orientation =*/orientation,
            });
            break;
        }

        case world::player_status: {
            fprintf(stderr, "FIXME: unhandled world::player_status\n");
            // unsigned pid = buffer.read<uint16_t>();
            // String name = buffer.readString();
            // unsigned status = buffer.read<uint16_t>();

            // game->playerStatus( pid, name, status );
            break;
        }

        case world::remove_world_obj: {
            fprintf(stderr, "FIXME: unhandled world::remove_world_obj\n");
            // float x = buffer.read<float>();
            // float y = buffer.read<float>();

            // game->removeWorldObj( x, y );
            break;
        }

        case world::server_message: {
            li::String message = buffer.readString();

            sub.getBroker().publish<RealmChatMessage>("$Server", message.c_str());
            break;
        }

        case world::spawn_world_obj: {
            fprintf(stderr, "FIXME: unhandled world::spawn_world_obj\n");
            // String name = buffer.readString();
            // float x = buffer.read<float>();
            // float y = buffer.read<float>();
            // float orientation = buffer.read<float>();

            // game->spawnWorldObj( name, x, y, orientation );
            break;
        }

        case world::sync_rq:
            buffer.clear();
            buffer.writeLE<uint16_t>(world::sync);
            socket->send(buffer);
            break;

        default:
            printf("WARNING: unknown message id %04X\n", messageID);
        }
    }
}
}
