/*
 * Tales of Lanthaia Client v3.0
 *
 * Data packing: bleb or zip
 * ECS: entt
 * Game UI: imgui?
 * I/O: littl TcpSocket for now
 * Rendering: RenderingKit, eventually bgfx as core part of framework
 * Scripting: ?
 */

#include <Gfx/RenderingSystem.hpp>
#include "TitleScene.hpp"

#include <framework/app.hpp>
#include <framework/engine.hpp>
#include <framework/messagequeue.hpp>
#include <framework/utility/errorbuffer.hpp>
#include <Res/ResourceHeightMap.hpp>

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#endif

namespace Client {
using std::make_shared;
using std::make_unique;
using std::unique_ptr;
using zfw::ErrorBuffer_t;
using zfw::IEngine;

static ErrorBuffer_t* g_eb;

zfw::IResourceManager2* globalResMgr;

static unique_ptr<IEngine> SysInit(int argc, char** argv) {
    auto engine = zfw::CreateEngine2(g_eb, 0, argc, argv);

    if (!engine->Startup()) {
        engine->DisplayError(g_eb, true);
        return {};
    }

    return engine;
}

static void GameMain(int argc, char** argv) {
    PubSub::Broker broker;

    auto engine = SysInit(argc, argv);

    if (!engine) {
        return;
    }

    auto eventQueue = zfw::MessageQueue::Create();

    globalResMgr = engine->CreateResourceManager2();
    globalResMgr->RegisterResourceClass<Obs::Res::ResourceHeightMap>();

    auto viewSystem = make_unique<RenderingSystem>(*globalResMgr);

    if (!viewSystem->Startup(engine.get(), eventQueue)) {
        engine->DisplayError(g_eb, true);
        return;
    }

    engine->ChangeScene(make_shared<TitleScene>(*engine, *eventQueue, broker, *viewSystem));
    engine->RunMainLoop();
}

ClientEntryPoint {
#if defined(_DEBUG) && defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc();
#endif

    zfw::ErrorBuffer::Create(g_eb);

    GameMain(argc, argv);

    zfw::ErrorBuffer::Release(g_eb);
    return 0;
}
}
