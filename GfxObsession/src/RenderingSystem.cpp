#include <Gfx/RenderingContext.hpp>
#include <Gfx/RenderingSystem.hpp>

#include <framework/broadcasthandler.hpp>
#include <framework/colorconstants.hpp>
#include <framework/componenttype.hpp>
#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/engine.hpp>
#include <framework/entityworld2.hpp>
#include <framework/errorcheck.hpp>
#include <framework/resourcemanager2.hpp>
#include <framework/utility/essentials.hpp>

#include <RenderingKit/Model.hpp>
#include <RenderingKit/utility/Camera.hpp>
#include <RenderingKit/utility/RKVideoHandler.hpp>

using namespace RenderingKit;
using glm::vec3;
using std::make_unique;
using std::optional;

namespace Obs::Gfx {

bool RenderingSystem::Startup(zfw::IEngine* sys, zfw::MessageQueue* eventQueue) {
    auto imh = sys->GetModuleHandler(true);

    this->rk.reset(RenderingKit::TryCreateRenderingKit(imh));
    zombie_ErrorCheck(rk);

    if (!rk->Init(sys)) {
        return false;
    }

    auto wm = rk->GetWindowManager();

    // TODO API: refactor stateful LoadDefaultSettings
    // TODO API: consider enabling anti-alias by default
    if (!wm->LoadDefaultSettings(nullptr) || !wm->ResetVideoOutput()) {
        return false;
    }

    this->rm = rk->StartupRendering(CoordinateSystem::rightHanded);
    zombie_ErrorCheck(rm);

    sys->SetVideoHandler(make_unique<RKVideoHandler>(rm, wm, eventQueue));

    zombie_ErrorCheck(bp2d.Init(rm));
    zombie_ErrorCheck(bp3d.Init(rm));

    font = rm->CreateFontFace("RenderingSystem font");
    font->Open("DejaVuSans.ttf", 16, IFontFace::FLAG_SHADOW);

    auto& bh = sys->GetBroadcastHandler();
    bh.SubscribeToComponentType<zfw::Model3D>(*this);
    bh.SubscribeToComponentType<zfw::Position>(*this);

    return true;
}

void RenderingSystem::AddCustomLayer(ICustomRenderLayer& layer, const char* name) {
    layers.emplace_back(RenderLayer {&layer, name});
}

void RenderingSystem::AddEntityWorldLayer(zfw::IEntityWorld2& world, const char* name) {
    zombie_assert(false);
}

void RenderingSystem::DrawWorld(optional<zfw::EntityHandle> playerEntity) {
    // No hello received? Nothing to do!

    // if ( !player )
    // {
    //     if ( shaders )
    //         renderProgram2D->use();

    //     uiFont->drawString( {10.0f, 10.0f}, "Entering world...", Colour( 0.8f, 0.9f, 1.0f ), 0 );
    //     return;
    // }

    auto& rm = *this->rm;
    rm.Clear(zfw::COLOUR_WHITE);

    // TODO API: it's too easy to forget something
    Camera cam(CoordinateSystem::leftHanded);
    cam.SetClippingDist(1.0f, 1000.0f);
    cam.SetVFov(3.14f * 0.25f);
    cam.SetPerspective();
    constexpr auto angle2 = zfw::f_pi / 5.0f;
    constexpr auto dist = 12.0f;

    auto position = playerEntity.has_value() ? playerEntity->GetComponent<zfw::Position>() : nullptr;

    if (position) {
        // player yaw
        auto angle = -glm::roll(position->rotation);

        cam.SetViewWithCenterDistanceYawPitch(
                position->pos + glm::vec3 { 0.0f, 0.0f, 1.6f }, dist, angle + zfw::f_pi, angle2);
    }
    else {
        cam.SetViewWithCenterDistanceYawPitch({}, dist, 0.0f, angle2);
    }
    rm.SetCamera(&cam);

    rm.SetRenderState(RK_DEPTH_TEST, 1);

    bp3d.DrawGridAround({}, { 1.0f, 1.0f, 0.0f }, { 20, 20 }, zfw::RGBA_BLACK);

        // Picking

//        pickingMatch.clear();
//
//        picking->begin();
//        graph->pick( picking );
//        picking->end( mouseX, mouseY );

//        String table = ( ( OrderedListNode* ) graph->getNode( "world_objs" ) )->getTable();

        // Render

//        if ( shaders )
//            renderProgram->use();

    // light->render();

    for (auto const& layer : layers) {
        if (layer.custom) {
            layer.custom->Render(*this);
        }
    }

//        water->renderBegin();
//        water->translate( Vector<float>( 0.0f, 0.0f, 0.0f ) );
        // water->render();
//        water->renderEnd();

        // Overlay

//        if ( shaders )
//            renderProgram2D->use();

//        gr->disableDepthTesting();
        // iterate ( players )
        //     players.current()->renderName();
//        gr->enableDepthTesting();

        rm.SetProjectionOrthoScreenSpace(-1.0f, 1.0f);
        rm.SetRenderState(RK_DEPTH_TEST, 0);
        // gr->pushBlendMode( IGraphicsDriver::additive );

        if (position) {
            RenderingContext ctx{rm};
            auto ctx2d = RenderingContext2D{ctx}.WithTextColor(zfw::RGBA_WHITE);

            ctx2d.WithAlignment({HAlignment::left, VAlignment::top}).WithFont(*font).WithPosLeftTop({5, 5}).FormatTextRow("Tales of Lanthaia (player {:.1f} {:.1f} {:.1f})", position->pos.x, position->pos.y, position->pos.z);
        }

        // if ( displayUi )
        //     uiFont->drawString( {displayMode.x - 10.0f, 10.0f}, ( String )"tolcl pre-Alpha -- " + player->loc.x + ", " + player->loc.y, Colour( 0.5f, 0.8f, 1.0f ), IFont::right );
        // else
        //     uiFont->drawString( {displayMode.x - 10.0f, 10.0f}, "Tales of Lanthaia pre-Alpha", Colour( 0.5f, 0.8f, 1.0f ), IFont::right );

//        chatFont->render( 8.0f, 8.0f, table + "\n\npicking: " + picking->getId() );

        // gr->popBlendMode();

        // if ( displayUi )
        // {
        //     for ( unsigned i = 0; i < chat.getLength(); i++ )
        //         chatFont->drawString( {20.0f, round( displayMode.y - 60.0f - i * chatFont->getLineSkip() * 1.2f )}, chat[chat.getLength() - i - 1], Colour::white(), 0 );

        //     if ( isTalking )
        //     {
        //         gr->drawRectangle( Vector<float>( 0.0f, displayMode.y - 30.0f ), Vector2<float>( displayMode.x, displayMode.y ), Colour( 0.0f, 0.0f, 0.0f, 0.6f ), nullptr );
        //         chatFont->drawString( {10, (float)displayMode.y - 15}, text + "+", Colour( 1.0f, 1.0f, 1.0f ), IFont::left | IFont::middle );
        //     }
        // }

//        if ( !pickingMatch.isEmpty() )
//            ui->showTooltip( mouseX + 40, mouseY, "\\w\\ Picking: \\s" + pickingMatch );
//        else
//            ui->hideTooltip();

        // if (ui) {
        //     ui->onRender();
        // }
}

void RenderingSystem::OnComponentEvent(zfw::IEntityWorld2& world, intptr_t entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) {
}

}
