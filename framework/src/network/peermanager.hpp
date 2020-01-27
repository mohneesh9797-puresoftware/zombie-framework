#ifndef framework_network_peermanager_hpp
#define framework_network_peermanager_hpp

#include <framework/base.hpp>

namespace zfw
{
    struct TcpHost {
        const char* hostname;
        uint16_t portNumber;
    };

    struct TcpPort {
        uint16_t portNumber;
    };

    // TODO: is a system?
    class IPeerManager {
        public:
            /**
             * Upon successful establishment of a connection, the IPeerSession is presented in an event,
             * as is a copy of the TcpHost. (if necessary, compare the hostname by equality, not identity!)
             *
             * Ownership of the session is retained by IPeerManager.
             * The session is destroyed upon calling DisconnectPeer.
             *
             * TODO: maybe allow requesting a shared_ptr?
             */
            void ConnectTo_Async(TcpHost host);

            void DisconnectPeer(IPeerSession* peer);
            bool ListenForConnections(TcpPort port);
    };

    class IPeerSession {
        public:
            void BindToWorld(IEntityWorld* world);
            void SetComponentSyncPolicy(IComponentType& type, bool send, bool receive, size_t bufferDepth);
            void SetEntitySyncPolicy(bool isThisAuthority, bool isPeerAuthority);
    };
}

#endif
