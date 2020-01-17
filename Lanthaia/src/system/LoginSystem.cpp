#include "LoginSystem.hpp"

using std::make_optional;
using std::nullopt;
using std::optional;

namespace Client
{

void LoginSession::ConnectTo(std::tuple<const char*, uint16_t> hostAndPort) {
    if (state == State::idle) {
        if (!socket) {
            socket = li::TcpSocket::create(false);
        }

        auto [host, port] = hostAndPort;
        socket->connect(host, port, false);

        SetState(State::connecting);
    }
    else {
        throw std::runtime_error("Invalid state of LoginSession");
    }
}

void LoginSession::Login(const std::string& username, const std::string& password) {
    if (state != State::readyToLogin) {
        throw std::runtime_error("Invalid state of LoginSession");
    }

    li::ArrayIOStream message;
    message.writeString("login.login_request");
    message.writeString(username.c_str());
    message.writeString(password.c_str());
    socket->send( message );

    SetState(State::loggingIn);
}

optional<LoginSession::StateUpdate> LoginSession::Update() {
    switch (state) {
        case State::connecting: {
            auto maybeSuccess = socket->connectFinished2();

            if (maybeSuccess.has_value()) {
                if (maybeSuccess->errno_ == 0) {
                    int clientVersion = 3;

                    // Send the hello
                    li::ArrayIOStream message;
                    message.writeString( "login.client_hello" );
                    message.writeLE<uint32_t>( clientVersion );
                    socket->send( message );

                    SetState(State::awaitingServerHello);
                    return StateUpdate {GetState(), ""};
                } else {
                    SetState(State::error);
                    return StateUpdate {GetState(), maybeSuccess->message};
                }
            }
            break;
        }

        case State::awaitingServerHello: {
            li::ArrayIOStream message;

            if (!socket->receive( message)) {
                break;
            }

            message.dump();
            auto messageName = message.readString();

            if ( messageName == "login.server_info" ) {
                auto realmName = message.readString();
                auto realmNews = message.readString();

                serverInfo = LoginServerInfo { realmName.c_str(), realmNews.c_str() };

                SetState(State::readyToLogin);
                return StateUpdate {GetState(), ""};
            }
            else if ( messageName == "login.server_down" ) {
                auto reason = message.readString();

                SetState(State::serverClosed);
                return StateUpdate {GetState(), reason.c_str()};
            }
            else {
//                setStatus( error, messageName );
//                listener->onLoginSessionStatus();
            }

            break;
        }

        case State::loggingIn: {
            li::ArrayIOStream message;

            if (!socket->receive( message)) {
                break;
            }

            message.dump();
            auto messageName = message.readString();

            if ( messageName == "result.ok" ) {
                SetState(State::loggedIn);
                return StateUpdate {GetState(), ""};
            }
            else {
                SetState(State::failed);
                return StateUpdate {GetState(), messageName.c_str()};
            }
            break;
        }
    }

    return nullopt;
}

LoginSystem::LoginSystem(PubSub::Broker& broker)
        : sub(broker, myPipe) {
    sub.add<LoginRequest>();
}

void LoginSystem::OnTicks(int ticks) {
    auto stateUpdate = session.Update();

    if (stateUpdate.has_value()) {
        sub.getBroker().publish<LoginSession::StateUpdate>(LoginSession::StateUpdate {*stateUpdate});

        if (stateUpdate->state == LoginSession::State::readyToLogin) {
            sub.getBroker().publish<LoginServerInfo>(LoginServerInfo { session.GetServerInfo() });
        }
    }

    switch (session.GetState()) {
        case LoginSession::State::idle: {
            // Parse Server URI
//            Uri::Parts uriParts;
//            Uri::parse( uri, uriParts );
//            uri.clear();
//            int port = uriParts.port.isEmpty() ? 24897 : uriParts.port.toInt();

            std::tuple<const char*, uint16_t> loginEndpoint{"127.0.0.1", 24897};
//            std::tuple<const char*, uint16_t> loginEndpoint{"google.com", 80};
//            std::tuple<const char*, uint16_t> loginEndpoint{"tsim.viewdns.net", 1000};
            session.ConnectTo(loginEndpoint);
            break;
        }

        default:
            ;
    }

    while (auto msg = myPipe.poll()) {
        if (auto loginRequest = msg->cast<LoginRequest>()) {
            session.Login(loginRequest->username, loginRequest->password);
        }
    }
}

}
