#ifndef LANTHAIA_LOGINSYSTEM_HPP
#define LANTHAIA_LOGINSYSTEM_HPP

#include <framework/system.hpp>
#include <PubSub.hpp>

#include <littl/TcpSocket.hpp>

namespace Client
{

struct LoginServerInfo {
    std::string realmName;
    std::string realmNews;
};

class LoginSession {
public:
    enum class State {
        idle,
        connecting,
        awaitingServerHello,
        readyToLogin,
        serverClosed,
        error,
//        registering,
        loggingIn,
        failed,
        loggedIn,
        enteringWorld,
    };

    struct StateUpdate { State state; std::string message; };

    [[nodiscard]] LoginServerInfo GetServerInfo() const { return serverInfo; }
    [[nodiscard]] State GetState() const { return state; }
    std::optional<StateUpdate> Update();

    void ConnectTo(std::tuple<const char*, uint16_t> hostAndPort);
    void Login(const std::string& username, const std::string& password);

private:
    void SetState(State state) { this->state = state; }

    State state = State::idle;
    LoginServerInfo serverInfo;
    std::string username;

    std::unique_ptr<li::TcpSocket> socket;
};

struct LoginRequest {
    std::string username;
    std::string password;
};

// publishes LoginServerInfo
// publishes LoginSession::StateUpdate
// subscribes LoginRequest
class LoginSystem : public zfw::ISystem {
public:
    explicit LoginSystem(PubSub::Broker& broker);

    void OnTicks(int ticks) override;

private:
    LoginSession session;

    PubSub::Pipe myPipe;
    PubSub::Subscription sub;
};

}

#endif
