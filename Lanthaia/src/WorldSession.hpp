#include <PubSub.hpp>

#include <framework/datamodel.hpp>

#include <littl/TcpSocket.hpp>

#include <string>

namespace Client {

using glm::vec3;
using std::unique_ptr;

struct RealmChatMessage {
    std::string sender; // TODO API: instead should be a reference to an entity, or $Server
    std::string message;
};

struct RealmMyCharacterInfo {
    int pid;
    std::string name;
    vec3 pos;
    float orientation;
};

struct RealmEntityAddition {
    int pid;
    std::string name;
    vec3 pos;
    float orientation;
};

struct RealmEntityRemoval {
    int pid;
};

struct RealmEntityPositionUpdate {
    int pid;
    vec3 pos;
    float orientation;
};

struct RealmSessionLost {};

class WorldSession {
public:
    WorldSession(PubSub::Broker& broker, unique_ptr<li::TcpSocket> socket);

    void Chat(const std::string& text);
    void movement(const zfw::Float3& pos, float angle);
    void Update();

private:
    PubSub::Subscription sub;
    PubSub::Pipe myPipe;

    unique_ptr<li::TcpSocket> socket;
    li::ArrayIOStream buffer;

    unsigned playerPid;
};

}
