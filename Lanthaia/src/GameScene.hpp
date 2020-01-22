#ifndef LANTHAIA_GAMESCENE_HPP
#define LANTHAIA_GAMESCENE_HPP

#include "data/Map.hpp"

#include <framework/entityworld2.hpp>
#include <framework/scene.hpp>
#include <framework/utility/keytracker.hpp>

#include <Gfx/Fwd.hpp>

#include <PubSub.hpp>

#include <littl/TcpSocket.hpp>

namespace Client {

using Obs::Gfx::RenderingSystem;
using std::unique_ptr;

//struct Player;

//    class Inventory;
class OrderedListNode;
class PickingListener;
class Scripting;
class Sector;
class WorldSession;

//    struct Binding
//    {
//        //int type;
//        unsigned short key;
//        String chat;
//    };

class GameScene : public zfw::IScene {
public:
    GameScene(PubSub::Broker& broker, zfw::IEngine& engine, zfw::MessageQueue& eventQueue, RenderingSystem& r,
        unique_ptr<li::TcpSocket> socket);
    ~GameScene();

    bool Init() override { return true; }
    void Shutdown() override {}

    bool AcquireResources() override { return true; }
    void DropResources() override {}

    void DrawScene() override;
    void OnFrame(double delta) override;

private:
    PubSub::Pipe myPipe;
    PubSub::Subscription sub;

    WorldSession* session;

    // Entities
    std::unique_ptr<zfw::IEntityWorld2> ew;

    // Scene Objects
    //    StormGraph::Camera* cam;
    //    StormGraph::IModel* water;

    // Camera
    float angle2, dist;

    // User Interface
    //    StormGraph::IGui* ui;
    //    Vector<unsigned short> displayMode;
    //    StormGraph::ITexture* overlay;
    //    bool displayUi;
    //    StormGraph::IFont* uiFont, * chatFont;

    // Players
    std::optional<zfw::EntityHandle> playerEntity;

    // Controls
    zfw::KeyTracker ks;
//    bool left, right, up, down, zin, zout;
    //    Array<unsigned short> keys;
    //    List<Binding> bindings;

    // Movement
//    bool hasMoved;
//    float runSpeed;

    // Chat
    unsigned maxChatLength = 666;
    li::List<std::string> chat;

    bool isTalking;
    std::string text;

    // View Panning
    //    bool viewDrag;
    //    Vector<int> viewDragOrigin;

    // Game Map
    Map* map = nullptr;

    // Render Programs
    //    IShaderProgram* renderProgram, * renderProgram2D;

    // Beta/WIP features
    //    ISceneGraph* graph;
    //        Inventory* inventory;
    //        Scripting* scripting;
    //    String emptyString;
    bool devMode = true;
    //        String pickingMatch;

        zfw::IEngine& engine;
        zfw::MessageQueue& eventQueue;
        RenderingSystem& r;
};

}

#endif
