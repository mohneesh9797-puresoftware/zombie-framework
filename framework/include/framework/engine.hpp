#pragma once

#include <framework/base.hpp>
#include <framework/errorcodes.hpp>

namespace zfw
{
    enum LogType_t
    {
        kLogNever = 0,
        kLogAlways = 1,
        kLogError,
        kLogWarning,
        kLogInfo,
        kLogDebugInfo,
    };

    // Sys::Init
    enum
    {
        kSysNoInitFileSystem = 1,

		/** Currently this affects the way error messages are displayed */
        kSysNonInteractive = 2,

#if ZOMBIE_API_VERSION >= 202001
        kSysNoDefaultFileSystem = 3,
#endif
    };

    class IEngine
    {
        public:
            // public variables:
            //  sys_tickrate                (int)

            virtual ~IEngine() {}

            virtual bool Init(ErrorBuffer_t* eb, int flags) = 0;
            virtual void Shutdown() = 0;

            virtual IEssentials*    GetEssentials() = 0;

            // Available before startup
            virtual IVarSystem*         GetVarSystem() = 0;

            virtual bool                Startup() = 0;

            // Core Handlers
            virtual IBroadcastHandler&  GetBroadcastHandler() = 0;
            virtual IEntityHandler*     GetEntityHandler(bool createIfNull) = 0;
            virtual IFileSystem*        GetFileSystem() = 0;
            virtual IMediaCodecHandler* GetMediaCodecHandler(bool createIfNull) = 0;
            virtual IModuleHandler*     GetModuleHandler(bool createIfNull) = 0;
            virtual IFSUnion*           GetFSUnion() = 0;
            virtual IVideoHandler*      GetVideoHandler() = 0;
            virtual void                SetVideoHandler(unique_ptr<IVideoHandler>&& handler) = 0;
            //virtual IEntityHandler* InitEntityHandler() = 0;
            //virtual IModuleHandler* InitModuleHandler() = 0;

            virtual shared_ptr<IFileSystem> CreateStdFileSystem(const char* basePath, int access) = 0;

#ifdef ZOMBIE_WITH_BLEB
            virtual shared_ptr<IFileSystem> CreateBlebFileSystem(const char* path, int access) = 0;
#endif

#ifdef ZOMBIE_WITH_LUA
            virtual shared_ptr<ILuaScriptContext> CreateLuaScriptContext() = 0;
#endif

            // Component systems
            virtual void                AddSystem(unique_ptr<ISystem> component) = 0;

            // Main loop
            virtual void ChangeScene(shared_ptr<IScene> scene) = 0;
            virtual void RunMainLoop() = 0;
            virtual void StopMainLoop() = 0;
            virtual void ReleaseScene() = 0;

            // Logging functions
            // TODO: this split is a long-standing mess. Just control the printing based on logType.
            // TODO API: make this available even without a reference to IEngine (either global or a reduced interface)
            virtual int Printf(LogType_t logType, const char* format, ...) = 0;
            virtual int Printfv(LogType_t logType, const char* format, va_list args) = 0;
            virtual void Log(LogType_t logType, const char* format, ...) = 0;
            virtual void Logv(LogType_t logType, const char* format, va_list args, size_t length) = 0;

            // Error handling functions
            virtual void DisplayError( const ErrorBuffer_t* eb, bool fatal ) = 0;
            virtual void PrintError( const ErrorBuffer_t* ebOrNull, LogType_t logType ) = 0;

            // Localization functions
            virtual const char* Localize(const char* text) = 0;

            // Utility
            virtual void DebugBreak(bool force) = 0;
            virtual void ParseArgs1(int argc, char* argv[]) = 0;        // will be reworked

            // I/O Utility
            // API TODO: return owning pointers!
            virtual bool CreateDirectoryRecursive(const char* path) = 0;
            virtual InputStream* OpenInput( const char* path ) = 0;
            virtual OutputStream* OpenOutput( const char* path ) = 0;

            // Timer management
            virtual uint64_t GetGlobalMicros() = 0;

            virtual int GetFrameCounter() = 0;
            virtual Profiler* GetProfiler() = 0;
            virtual bool IsProfiling() = 0;
            virtual void ProfileFrame(int frameNumber) = 0;

            // Utility Classes
            virtual IResourceManager*   CreateResourceManager(const char* name) = 0;
            virtual IResourceManager2*  CreateResourceManager2() = 0;
            virtual ShaderPreprocessor* CreateShaderPreprocessor() = 0;
            virtual Timer*              CreateTimer() = 0;
    };

    IEngine* CreateEngine();
    std::unique_ptr<IEngine> CreateEngine2(ErrorBuffer_t* eb, int initFlags, int argc, char** argv);
}
