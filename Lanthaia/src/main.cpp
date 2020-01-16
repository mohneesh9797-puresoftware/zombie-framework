/*
 * Tales of Lanthaia Client v3.0
 *
 * Data packing: bleb or zip
 * ECS: entt
 * Game UI: ?
 * I/O: littl TcpSocket for now
 * Rendering: RenderingKit, migrate to bgfx-based solution?
 * Scripting: ?
 */

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#endif

#include <framework/app.hpp>
#include <framework/engine.hpp>
#include <framework/utility/errorbuffer.hpp>

namespace Client
{
    using std::unique_ptr;
    using zfw::ErrorBuffer_t;
    using zfw::IEngine;

    static ErrorBuffer_t* g_eb;

    static unique_ptr<IEngine> SysInit(int argc, char** argv)
    {
        auto engine = zfw::CreateEngine2(g_eb, 0, argc, argv);

        if (!engine->Startup()) {
            engine->DisplayError(g_eb, true);
            return {};
        }

        return engine;
    }

    static void GameMain(int argc, char** argv)
    {
        auto engine = SysInit(argc, argv);

        if (!engine) {
            return;
        }
    }

    ClientEntryPoint
    {
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
