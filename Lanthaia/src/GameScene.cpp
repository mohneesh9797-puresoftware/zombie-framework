#include "GameScene.hpp"
#include "WorldSession.hpp"
#include "rendering/Ms3dModelLayer.hpp"
#include "rendering/TerrainRenderLayer.hpp"

#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/engine.hpp>
#include <framework/entityworld2.hpp>
#include <framework/event.hpp>
#include <framework/messagequeue.hpp>

namespace Client {

using namespace zfw;
using std::make_unique;
using std::move;

void GameScene::DrawScene() {
    while (chat.getLength() > maxChatLength)
        chat.remove(0);

    r.DrawWorld(playerEntity);
}

enum BindingIndices {
    moveUpKey,
    moveDownKey,
    moveLeftKey,
    moveRightKey,
    turnLeftKey,
    turnRightKey,
    chatKey,
    hideUiKey,
    /****/ escKey,
    numEnterKey,
    toggleShadersKey,
    inventoryKey
};

struct Binding {
    int id;
    const char* bindingName;
    Vkey_t vkey;
};

static constexpr Binding bindings[] = {
    { moveUpKey, "move_up", Vkey_t { VKEY_KEY, -1, 'w', 0 } },
    { moveDownKey, "move_down", Vkey_t { VKEY_KEY, -1, 's', 0 } },
    { moveLeftKey, "move_left", Vkey_t { VKEY_KEY, -1, 'a', 0 } },
    { moveRightKey, "move_right", Vkey_t { VKEY_KEY, -1, 'd', 0 } },
    { turnLeftKey, "turn_left", Vkey_t { VKEY_KEY, -1, 'q', 0 } },
    { turnRightKey, "turn_right", Vkey_t { VKEY_KEY, -1, 'e', 0 } },
};

// Test code
//    static ILight* light;
//    static bool shaders = true;

//    static int mouseX, mouseY;

// static void loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned
// count )
// {
//     cfx2_Node* doc = sg->loadCfx2Asset( fileName );

//     for ( unsigned i = 0; i < count; i++ )
//         values[i] = sg->getGraphicsDriver()->getKey( cfx2_query_value( doc, ( String )"Controls/" + bindingNames[i] )
//         );

//     cfx2_release_node( doc );
// }

unique_ptr<Ms3dModelLayer> my_layer;
unique_ptr<TerrainRenderLayer> my_layer2;
extern zfw::IResourceManager2* globalResMgr;

GameScene::GameScene(PubSub::Broker& broker, zfw::IEngine& engine, zfw::MessageQueue& eventQueue, RenderingSystem& r,
    unique_ptr<li::TcpSocket> socket)
    : sub(broker, myPipe)
    , ew(IEntityWorld2::Create(engine.GetBroadcastHandler()))
    ,
    // ui( 0 ), overlay( 0 ), displayUi( false ),
    ks(engine.GetVarSystem())
    ,
    //            hasMoved( false ),
    isTalking(false)
    // viewDrag( false ), map( 0 ), renderProgram( 0 ), renderProgram2D( 0 )
    , engine(engine)
    , eventQueue(eventQueue)
    , r(r) {
    for (auto const& binding : bindings) {
        ks.Subscribe(binding.id, binding.bindingName, binding.vkey);
    }

    globalResMgr->SetTargetState(zfw::IResource2::REALIZED);
    r.GetRm().RegisterResourceProviders(globalResMgr);

    my_layer = std::make_unique<Ms3dModelLayer>(engine.GetBroadcastHandler(), engine, *ew, *globalResMgr, r.GetRm());
    r.AddCustomLayer([](auto& ctx) { my_layer->Render(ctx); }, "ms3dModelLayer");

    my_layer2 = std::make_unique<TerrainRenderLayer>(engine.GetBroadcastHandler(), engine, *ew, *globalResMgr);
    r.AddCustomLayer([](auto& ctx) { my_layer2->Render(ctx); }, "terrainRenderLayer");

    //         loadKeyBindings( keys, "profile/default.tolcl/controls.cfx2", bindingNames, sizeof( bindingNames ) /
    //         sizeof( *bindingNames ) ); auto gr = sg->getGraphicsDriver(); keys[escKey] = gr->getKey( "Escape" );
    //         keys[numEnterKey] = gr->getKey( "Num Enter" );
    //         keys[toggleShadersKey] = gr->getKey( "G" );
    //         keys[inventoryKey] = gr->getKey( "I" );

    //         auto waterTex = globalResMgr->getTexture("tolcl/tex/0_water.png");
    //         IMaterial* mat = gr->createSolidMaterial( "water", Colour::white()/*, waterTex*/ ,nullptr );

    //         //IHeightMap* flat = sg->loa HeightMap( "tolcl/heightmap/flat.png", Vector<float>( 1000.0f, 1000.0f ),
    //         0.0f ); li::Object<IHeightMap> flat = sg->createHeightMap( {2, 2} ); TerrainCreationInfo terrain {
    //                 flat,
    //                 Vector<float>( 50.0f, 50.0f, 0.0f ),
    //                 Vector<>( 0.0f, 0.0f, 0.0f ),
    //                 flat->getResolution(),
    //                 Vector2<>(),
    //                 Vector2<float>( 50.0f, 50.0f ),
    //                 true,
    //                 true,
    //                 false,
    //                 mat
    //         };
    // //        water = gr->createTerrain( "water", &terrain, 0 );
    //         PlaneCreationInfo pci {
    //                 {300.0f, 300.0f},
    //                 {0.0f, 0.0f},
    //                 {0.0f, 0.0f},
    //                 {50.0f, 50.0f},
    //                 false, true, mat
    //         };
    //         water = gr->createPlane("water", &pci);
    //         flat.release();

    //         IResourceManager* uiResMgr = Resources::getUiResMgr();
    //         uiFont = uiResMgr->getFont( "Radiance.EpicStyler.Assets/DefaultFont.ttf", 20, IFont::normal );
    //         chatFont = uiFont;

    // //        if ( shaders )
    // //        {
    // //            renderProgram = new Program( "sg_assets/shader/Default" );
    // //            renderProgram2D = new Program( "sg_assets/shader/2D" );
    // //
    // //            renderProgram->use();
    // //        }

    //         gr->setSceneAmbient( Colour( 0.2f, 0.2f, 0.2f ) );

    //         displayMode = gr->getWindowSize();
    //         maxChatLength = unsigned( ( displayMode.y - 100.0f ) / 25.0f );

    session = new WorldSession(broker, move(socket));

    //         //light = new Light( Light_positional, Vector<float>( 300.0f, 300.f, 30.f ), Vector<float>(), Colour(
    //         0.2f, 0.2f, 0.2f ), Colour( 1.0f, 1.0f, 1.0f ), 40.0f ); light = gr->createDirectionalLight(
    //         Vector<float>( 0.0f, 0.0f, -1.0f ), Colour(), Colour( 0.5f, 0.5f, 0.5f ), 40.0f );

    // //        inventory = new Inventory();

    //         ui = sg->getGuiDriver()->createGui({}, Vector<>(displayMode.x, displayMode.y));

    // //        IPanel* mainPanel = ui->createPanel( Vector<>(displayMode.x / 2 + 40, displayMode.y - 48), {400, 48} );
    // //        GameUI::GraphicalButton* invButton = new GameUI::GraphicalButton( 8, 8, 32, 32,
    // globalResMgr->getTexture( "tolcl/gfx/invbutton.png" ) );
    // //        invButton->setName( "inv_button" );
    // //        invButton->setOnPush( this );
    // //        mainPanel->add( invButton );
    // //        ui->add( mainPanel );

    // //        GameUI::ItemDrag* drag = new GameUI::ItemDrag( ui );
    // //        drag->setName( "item_drag" );
    // //        ui->add( drag );
    // //        ui->overlay( drag );

    //         up = down = left = right = zin = zout = false;

    //         // init scripting engine

    // //        scripting = new Scripting( this );
    // //        InputStream* autorun = sg->open( "autorun.misl" );
    // //
    // //        if ( autorun )
    // //            scripting->execute( autorun );
    // //
    //         graph = sg->createSceneGraph("world");

    sub.add<RealmMyCharacterInfo>();
}

GameScene::~GameScene() {
    //     delete renderProgram;
    //     delete renderProgram2D;

    //     delete map;

    //     water->release();

    //     if ( graph )
    //         delete graph;

    //     iterate ( players )
    //     delete players.current();
    // player_refs
}

void GameScene::OnFrame(double delta) {
    ks.OnFrame();

    if (session) {
        session->Update();
    }

    while (auto msg = myPipe.poll()) {
        if (auto myCharInfo = msg->cast<RealmMyCharacterInfo>()) {
            map = new Map(myCharInfo->pos.x, myCharInfo->pos.y, engine, *ew);

            playerEntity = ew->CreateEntityWithId(myCharInfo->pid);
            playerEntity->SetComponent(zfw::Model3D { "tolcl/model/human_0_sny.ms3d" });
            playerEntity->SetComponent(
                zfw::Position { myCharInfo->pos, glm::angleAxis(myCharInfo->orientation, vec3 { 0.0f, 0.0f, 1.0f }) });

            // displayUi = true;
        }
        else if (auto entInfo = msg->cast<RealmEntityAddition>()) {
            auto entity = ew->CreateEntity();
            entity.SetComponent(zfw::Model3D { "tolcl/model/human_0_sny.ms3d" });
            entity.SetComponent(
                zfw::Position { entInfo->pos, glm::angleAxis(entInfo->orientation, vec3 { 0.0f, 0.0f, 1.0f }) });
        }
    }

    while (MessageHeader* msg = eventQueue.Retrieve(li::Timeout(0))) {
        switch (msg->type) {
        case EVENT_WINDOW_CLOSE:
            engine.StopMainLoop();
            break;

        case EVENT_VKEY: {
            auto ev = msg->Data<EventVkey>();

            ks.Handle(*ev);

            if (ev->input.vkey.type == VKEY_KEY && ev->input.vkey.key == KEY_ESCAPE) {
                engine.StopMainLoop();
            }

            break;
        }
        }

        msg->Release();
    }

    auto playerPos = playerEntity.has_value() ? playerEntity->GetComponent<zfw::Position>() : nullptr;

    if (playerPos != nullptr && map != nullptr) {
        bool playerHasMoved = false;
        auto newPos = playerPos->pos;

        // glm's roll is our yaw. One day this might be fixed. Also why negated?
        auto angle = -glm::roll(playerPos->rotation);

        constexpr float turnSpeed = 3.0f;
        constexpr float runSpeed = 5.0f;

        if (ks.IsHeld(turnLeftKey)) {
            angle += turnSpeed * delta;
            playerHasMoved = true;
        }
        else if (ks.IsHeld(turnRightKey)) {
            angle -= turnSpeed * delta;
            playerHasMoved = true;
        }

        if (ks.IsHeld(moveUpKey)) {
            newPos += vec3(std::cos(angle) * runSpeed * delta, -std::sin(angle) * runSpeed * delta, 0.0f);
            playerHasMoved = true;
        }
        else if (ks.IsHeld(moveDownKey)) {
            newPos -= vec3(std::cos(angle) * runSpeed * delta, -std::sin(angle) * runSpeed * delta, 0.0f);
            playerHasMoved = true;
        }

        if (ks.IsHeld(moveLeftKey)) {
            newPos += vec3(std::cos(angle + zfw::f_pi / 2.0f) * runSpeed * delta,
                -std::sin(angle + zfw::f_pi / 2.0f) * runSpeed * delta, 0.0f);
            playerHasMoved = true;
        }
        else if (ks.IsHeld(moveRightKey)) {
            newPos += vec3(std::cos(angle - zfw::f_pi / 2.0f) * runSpeed * delta,
                -std::sin(angle - zfw::f_pi / 2.0f) * runSpeed * delta, 0.0f);
            playerHasMoved = true;
        }

        if (playerHasMoved) {
            newPos.z = map->getHeightAt(newPos.x, newPos.y);
            session->movement(newPos, angle);
            map->moveCenter(engine, *ew, newPos.x, newPos.y);

            playerEntity->SetComponent(zfw::Position { newPos, glm::angleAxis(-angle, vec3 { 0.0f, 0.0f, 1.0f }) });
        }
    }
}

// void GameScene::bind( const String& keyName, const String& event )
// {
//     unsigned short key = sg->getGraphicsDriver()->getKey( keyName );

//     reverse_iterate ( bindings )
//     if ( key == bindings.current().key )
//     {
//         bindings.remove( bindings.iter() );
//         break;
//     }

//     Binding binding = { key, event };
//     bindings.add( binding );
// }

// bool GameScene::closeButtonAction()
// {
//     if ( !isTalking )
//         return true;
//     else
//         return false;
// }

// void GameScene::doMovement( float delta )
// {
//     if ( up )
//     {
//         player->loc += Vector<float>( cos( player->angle ) * runSpeed * delta, -sin( player->angle ) * runSpeed *
//         delta ); hasMoved = true;
//     }
//     else if ( down )
//     {
//         player->loc -= Vector<float>( cos( player->angle ) * runSpeed * delta, -sin( player->angle ) * runSpeed *
//         delta ); hasMoved = true;
//     }

//     if ( left )
//     {
//         player->loc += Vector<float>( cos( player->angle + M_PI / 2.0f ) * runSpeed * delta, -sin( player->angle +
//         M_PI / 2.0f ) * runSpeed * delta ); hasMoved = true;
//     }
//     else if ( right )
//     {
//         player->loc += Vector<float>( cos( player->angle - M_PI / 2.0f ) * runSpeed * delta, -sin( player->angle -
//         M_PI / 2.0f ) * runSpeed * delta ); hasMoved = true;
//     }
// }

// void GameScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
// {
//     if ( isTalking && state == Key::pressed )
//     {
//         if ( key == 0x08 )
//             text = text.dropRightPart( 1 );
//         else if ( key == 0x0D || key == keys[escKey] || key == keys[numEnterKey] )
//         {
//             if ( key != keys[escKey] )
//                 say( text );

//             isTalking = false;

//             text.clear();
//         }
//         else if ( character >= 0x20 )
//             text = text + Utf8Character( character );

//         return;
//     }

//     if ( state == Key::pressed )
//     {
//         iterate ( bindings )
//         if ( key == bindings.current().key )
//         {
//             session->chat( bindings.current().chat );
//             return;
//         }
//     }

//     if ( key == keys[moveUpKey] )
//         up = (state == Key::pressed);
//     else if ( key == keys[moveDownKey] )
//         down = (state == Key::pressed);
//     else if ( key == keys[moveLeftKey] )
//         left = (state == Key::pressed);
//     else if ( key == keys[moveRightKey] )
//         right = (state == Key::pressed);
//     else if ( key == keys[chatKey] && state == Key::pressed )
//         isTalking = true;
//     else if ( key == keys[hideUiKey] && state == Key::pressed )
//     {
//         displayUi = !displayUi;
//         hasMoved = true;
//     }
//     else if ( key == keys[toggleShadersKey] && state == Key::pressed )
//     {
//         // TODO
//         shaders = !shaders;
//         //sg->detachShader();
//     }
//     else if ( key == keys[inventoryKey] && state == Key::pressed )
//     {
//     }

//     if ( devMode && key == /*Key::mouseWheelUp*/ -4 && state == Key::pressed ) {
//         dist -= 1.0f;
//     }

//     if ( devMode && key == /*Key::mouseWheelDown*/ -5 && state == Key::pressed ) {
//         dist += 1.0f;
//     }
// }

//     void GameScene::mouseButton( int x, int y, bool right, bool down )
//     {
//         if ( !right )
//         {
// //            if ( down )
// //                ui->mouseDown( x, y );
// //            else
// //                ui->mouseUp( x, y );
//         }
//         else
//         {
//             viewDrag = down;
//             viewDragOrigin = Vector<int>( x, y );
//         }
//     }

//     void GameScene::onMouseMoveTo( const Vector2<int>& mouse )
//     {
//         mouseX = mouse.x;
//         mouseY = mouse.y;
// //        ui->mouseMove( x, y );

//         if ( player && viewDrag )
//         {
//             if ( devMode )
//                 angle2 = angle2 + ( mouse.y - viewDragOrigin.y ) / 400.0f;

//             player->angle -= ( mouse.x - viewDragOrigin.x ) / 400.0f;

//             if ( mouse.x != viewDragOrigin.x )
//                 hasMoved = true;

//             viewDragOrigin = Vector<int>( mouse.x, mouse.y );
//         }
//     }

//     void GameScene::newPlayer( unsigned pid, const String& name, const Vector<float>& loc, float angle )
//     {
//         players.add( new Player( pid, name, loc, angle ) );
// player_refs
//     }

//     void GameScene::onPickingMatch( ObjectNode* node )
//     {
// //        pickingMatch = node->getName();
//     }

//     void GameScene::parseClientCommand( const String& text )
//     {
//         List<String> tokens;

//         text.parse( tokens, ' ', '\\' );

//         if ( false ) {}
// //        if ( tokens[0] == "exec" )
// //        {
// //            InputStream* input = File::open( tokens[1] );
// //
// //            if ( !input || !scripting->execute( input ) )
// //                write( "\\r" "Script failed!" );
// //        }
// //        else if ( tokens[0] == "export" && player )
// //        {
// //            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );
// //
// //            if ( sect )
// //                sect->saveAs( tokens[1] );
// //        }
// //        else if ( tokens[0] == "give" )
// //        {
// //            unsigned bag, slot;
// //
// //            if ( !inventory->getEmptySlot( bag, slot ) )
// //                write( "\\r" "No empty slot found." );
// //            else
// //                inventory->set( bag, slot, Item( tokens[1] ) );
// //        }
// //        else if ( tokens[0] == "inv" )
// //        {
// //            unsigned numBags = inventory->getNumBags();
// //
// //            for ( unsigned bag = 0; bag < numBags; bag++ )
// //            {
// //                unsigned numSlots = inventory->getNumSlots( bag );
// //                write( ( String )"\\S\\g" "Bag " + bag + ": " + numSlots + " slots" );
// //
// //                for ( unsigned i = 0; i < numSlots; i++ )
// //                {
// //                    Slot* slot = inventory->get( bag, i );
// //
// //                    if ( slot && !slot->isEmpty() )
// //                        write( ( String )"\\l" " -- " + bag + "." + i + ": " + slot->get().getName() );
// //                }
// //            }
// //        }
// //        else if ( tokens[0] == "savesect" && player )
// //        {
// //            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );
// //
// //            if ( sect )
// //                sect->save();
// //        }
//         else if ( tokens[0] == "setmodel" && player )
//             player->changeModel( tokens[1] );
//         else if ( tokens[0] == "tp" && player )
//         {
//             player->loc = Vector<float>( tokens[1], tokens[2] );
//             hasMoved = true;
//         }
//         else if ( tokens[0] == "ubind" )
//         {
//             unsigned short key = sg->getGraphicsDriver()->getKey( tokens[1] );

//             iterate ( bindings )
//             if ( key == bindings.current().key )
//             {
//                 bindings.remove( bindings.iter() );
//                 break;
//             }
//         }
//     }

//     void GameScene::playerLocation( unsigned pid, const Vector<float>& loc, float angle )
//     {
//         iterate ( players )
//         if ( players.current()->pid == pid )
//         {
//             players.current()->loc = loc;
//             players.current()->angle = angle;
//             return;
//         }
//     }

//     void GameScene::playerStatus( unsigned pid, const String& name, unsigned status )
//     {
//         if ( status == status::offline )
//         {
//             write( name + "\\o" " left the game." );

//             iterate ( players )
//             if ( players.current()->pid == pid )
//             {
//                 Player* p = players.current();
//                 players.remove( players.iter() );
// player_refs
//                 delete p;
//                 return;
//             }
//         }
//         else if ( status == status::online )
//             write( name + "\\g" " connected." );
//     }

//     void GameScene::removePlayer( unsigned pid )
//     {
//         iterate ( players )
//         if ( players.current()->pid == pid )
//         {
//             delete players.current();
//             players.remove( players.iter() );
// player_refs
//             return;
//         }
//     }

//     void GameScene::removeWorldObj( float x, float y )
//     {
//         Sector* sect = map->getSectorAt( x, y );

//         if ( sect )
//             sect->deleteWorldObj( x, y );
//     }

//     void GameScene::onUpdate( double delta )
//     {
//         // TODO: remove one time
//         // Lock map object (mthread sort())
//         // MUST BE DONE AND MUST BE DONE FIRST
//         // otherwise deadlock

//         // Handle network I/O

//         session->process();

//         // Lock the self-sorting world map

//         if ( map )
//             map->lock();

//         // Movement

//         doMovement( delta );

//         // Movement sync

//         if ( hasMoved )
//         {
//             player->loc.z = map->getHeightAt( player->loc.x, player->loc.y );
//             session->movement( player->loc, player->angle );
//             map->moveCenter( player->loc.x, player->loc.y );
//             hasMoved = false;
//         }

//         if (ui) {
//             ui->onUpdate(delta);
//         }
//     }

//     void GameScene::say( const String& text )
//     {
//         if ( !text.isEmpty() )
//         {
//             if ( text.beginsWith( "#" ) )
//                 parseClientCommand( text.dropLeftPart( 1 ) );
// //            else if ( text.beginsWith( "$" ) )
// //                scripting->execute( new ArrayIOStream( text.dropLeftPart( 1 ) ) );
//             else
//                 session->chat( text );
//         }
//     }

//     void GameScene::setDevMode( bool enabled )
//     {
//         devMode = enabled;
//     }

//     void GameScene::spawnWorldObj( const String& name, float x, float y, float orientation )
//     {
//         Sector* sect = map->getSectorAt( x, y );

//         if ( sect )
//             sect->addWorldObj( name, x, y, map->getHeightAt( x, y ), orientation );
//     }

// //    void GameScene::uiEvent( GameUI::Widget* widget, const String& event )
// //    {
// //        if ( widget->getName() == "inv_button" && event == "push" )
// //        {
// //            GameUI::InventoryWindow* inv = new GameUI::InventoryWindow( ui, 50, 50, inventory->getBag( 0 ) );
// //            inv->setName( "inventory_window" );
// //            inv->setOnClose( this );
// //            ui->add( inv );
// //        }
// //        else if ( widget->getName() == "inventory_window" && event == "close" )
// //            ui->destroyWidget( widget );
// //    }

//     void GameScene::write( const String& text )
//     {
//         chat.add( text );
//     }
}
