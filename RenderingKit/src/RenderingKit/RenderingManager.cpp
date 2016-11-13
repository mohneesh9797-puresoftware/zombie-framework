
#include "RenderingKitImpl.hpp"
#include <RenderingKit/RenderingKitUtility.hpp>

#include <framework/resourcemanager.hpp>
#include <framework/timer.hpp>

#include <gameui/uithemer.hpp>

#include <littl/Stack.hpp>

namespace RenderingKit
{
    using namespace zfw;

    // FIXME: Bounds checking for all usages
    static const GLenum primitiveTypeToGLMode[] = { GL_LINES, GL_TRIANGLES, GL_QUADS };

    // ====================================================================== //
    //  class declaration(s)
    // ====================================================================== //

    class RenderingManager : public IRenderingManagerBackend, public IVertexCacheOwner, public zfw::IResourceProvider
    {
        public:
            RenderingManager(zfw::ErrorBuffer_t* eb, RenderingKit* rk);
            ~RenderingManager();

            virtual void RegisterResourceProviders(zfw::IResourceManager* res) override;
            virtual zfw::IResourceManager* GetSharedResourceManager() override;

            virtual void BeginFrame() override;
            virtual void Clear() override;
            virtual void ClearDepth() override;
            virtual void EndFrame(int ticksElapsed) override;
            virtual void SetClearColour(const Float4& colour) override;

            virtual shared_ptr<ICamera>        CreateCamera(const char* name) override;
            virtual shared_ptr<IDeferredShadingManager>    CreateDeferredShadingManager() override;
            virtual shared_ptr<IFontFace>      CreateFontFace(const char* name) override;
            virtual shared_ptr<IFPMaterial>    CreateFPMaterial(const char* name, int flags) override;
            virtual shared_ptr<IGeomBuffer>    CreateGeomBuffer(const char* name) override;
            virtual zfw::shared_ptr<IGraphics> CreateGraphicsFromTexture(shared_ptr<ITexture> texture) override;
            virtual zfw::shared_ptr<IGraphics> CreateGraphicsFromTexture2(shared_ptr<ITexture> texture, const Float2 uv[2]) override;
            virtual shared_ptr<IMaterial>      CreateMaterial(const char* name, shared_ptr<IShader> program) override;
            virtual shared_ptr<IRenderBuffer>  CreateRenderBuffer(const char* name, Int2 size, int flags) override;
            virtual shared_ptr<IRenderBuffer>  CreateRenderBufferWithTexture(const char* name, shared_ptr<ITexture> texture, int flags) override;
            virtual shared_ptr<ITexture>       CreateTexture(const char* name) override;
            virtual shared_ptr<ITextureAtlas>  CreateTextureAtlas2D(const char* name, Int2 size) override;

            virtual shared_ptr<IVertexFormat>  CompileVertexFormat(IShader* program, uint32_t vertexSize,
                    const VertexAttrib_t* attributes, bool groupedByAttrib) override;

            virtual void DrawPrimitives(IMaterial* material, RKPrimitiveType_t primitiveType, IGeomChunk* gc) override;

            virtual Int2 GetViewportSize() override { return viewportSize; }
            virtual void GetViewportPosAndSize(Int2* viewportPos_out, Int2* viewportSize_out) override;
            virtual void PopRenderBuffer() override;
            virtual void PushRenderBuffer(IRenderBuffer* rb) override;
            virtual bool ReadPixel(Int2 posInFB, Byte4* colour_outOrNull, float* depth_outOrNull) override;
            virtual void SetCamera(ICamera* camera) override;
            virtual void SetMaterialOverride(IMaterial* materialOrNull) override;
            virtual void SetProjection(const glm::mat4x4& projection) override;
            virtual void SetRenderState(RKRenderStateEnum_t state, int value) override;
            virtual void SetProjectionOrthoScreenSpace(float znear, float zfar) override;
            virtual void SetViewportPosAndSize(Int2 viewportPos, Int2 viewportSize) override;

            virtual void FlushMaterial(IMaterial* material) override { VertexCacheFlush(); }
            virtual void FlushCachedMaterialOptions() override { VertexCacheFlush(); }
            virtual void* VertexCacheAlloc(IVertexFormat* vertexFormat, IMaterial* material,
                    RKPrimitiveType_t primitiveType, size_t numVertices) override;
            virtual void* VertexCacheAlloc(IVertexFormat* vertexFormat, IMaterial* material,
                    const MaterialSetupOptions& options, RKPrimitiveType_t primitiveType, size_t numVertices) override;
            virtual void VertexCacheFlush() override;

            // RenderingKit::IRenderingManagerBackend
            virtual bool Startup() override;
            virtual bool CheckErrors(const char* caller) override;

            virtual void CleanupMaterialAndVertexFormat() override;
            virtual void OnWindowResized(Int2 newSize) override;
            virtual void SetupMaterialAndVertexFormat(IGLMaterial* material, const MaterialSetupOptions& options,
                GLVertexFormat* vertexFormat, GLuint vbo) override;

            // RenderingKit::IVertexCacheOwner
            virtual void            OnVertexCacheFlush(IGLVertexCache* vc, size_t bytesUsed) override;

            // zfw::IResourceProvider
            virtual shared_ptr<IResource> CreateResource(IResourceManager* res, const std::type_index& resourceClass,
                    const char* normparams, int flags) override;
            virtual bool            DoParamsAlias(const std::type_index& resourceClass, const char* params1,
                    const char* params2) override { return false; }
            virtual const char*     TryGetResourceClassName(const std::type_index& resourceClass) override;

        private:
            // State (objects)
            Stack<IGLRenderBuffer*> renderBufferStack;
            unique_ptr<zfw::IResourceManager> sharedResourceManager;

            struct VertexCache_t
            {
                unique_ptr<IGLVertexCache> cache;

                GLVertexFormat* vertexFormat;
                IGLMaterial* material;
                MaterialSetupOptions options;
                RKPrimitiveType_t primitiveType;
                size_t numVertices;
            };

            // State (POD)
            Int2 windowSize, framebufferSize;
            Int2 viewportPos, viewportSize;

            glm::mat4x4* projectionCurrent, * modelViewCurrent;

            void p_SetRenderBuffer(IGLRenderBuffer* rb);


            zfw::ErrorBuffer_t* eb;
            RenderingKit* rk;

            unique_ptr<zfw::Timer> fpsTimer;
            int fpsNumFrames;

            // Private resources
            shared_ptr<ICamera> cam;       // used for shortcut methods (SetProjectionOrtho)
            VertexCache_t vertexCache;

            // Moar State
            GLVertexFormat* currentVertexFormat;

            // Material Override
            IGLMaterial* materialOverride;
    };

    // ====================================================================== //
    //  class RenderingManager
    // ====================================================================== //

    IRenderingManagerBackend* CreateRenderingManager(zfw::ErrorBuffer_t* eb, RenderingKit* rk)
    {
        return new RenderingManager(eb, rk);
    }

    RenderingManager::RenderingManager(ErrorBuffer_t* eb, RenderingKit* rk)
    {
        this->eb = eb;
        this->rk = rk;

        fpsTimer.reset(rk->GetSys()->CreateTimer());

        cam = p_CreateCamera(eb, rk, this, "RenderingManager/cam");

        vertexCache.vertexFormat = nullptr;
        vertexCache.material = nullptr;

        currentVertexFormat = nullptr;
        materialOverride = nullptr;
    }

    RenderingManager::~RenderingManager()
    {
        sharedResourceManager.reset();
    }

    void RenderingManager::BeginFrame()
    {
        //if (profileFrame == frameNumber)
        //    profiler->ResetProfileRange();

        if (!fpsTimer->IsStarted())
        {
            fpsTimer->Start();
            fpsNumFrames = 0;
        }
        else
        {
            fpsNumFrames++;

            if (fpsTimer->GetMicros() >= 100000)
            {
                float frameTimeMs = fpsTimer->GetMicros() / 1000.0f / fpsNumFrames;
                float fps = 1000.0f / (frameTimeMs);

                char buffer[200];
                snprintf(buffer, sizeof(buffer), "%g fps @ %g ms/frame", glm::round(fps), frameTimeMs);

                rk->GetWindowManagerBackend()->SetWindowCaption(buffer);

                fpsTimer->Reset();
                fpsNumFrames = 0;
            }
        }

        GLStateTracker::ClearStats();
        SetViewportPosAndSize(Int2(), framebufferSize);
    }

    bool RenderingManager::CheckErrors(const char* caller)
    {
        GLenum err = glGetError();

        if (err != GL_NO_ERROR)
        {
            rk->GetSys()->Printf(kLogError, "OpenGL error %04Xh, detected in %s\n", err, caller);
            return false;
        }

        return true;
    }

    void RenderingManager::CleanupMaterialAndVertexFormat()
    {
        currentVertexFormat->Cleanup();
    }

    void RenderingManager::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void RenderingManager::ClearDepth()
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    shared_ptr<IVertexFormat> RenderingManager::CompileVertexFormat(IShader* program, uint32_t vertexSize,
            const VertexAttrib_t* attributes, bool groupedByAttrib)
    {
        auto fmt = std::make_shared<GLVertexFormat>(rk);
        
        fmt->Compile(program, vertexSize, attributes, groupedByAttrib);

        return fmt;
    }

    shared_ptr<ICamera> RenderingManager::CreateCamera(const char* name)
    {
        return p_CreateCamera(eb, rk, this, name);
    }

    shared_ptr<IDeferredShadingManager> RenderingManager::CreateDeferredShadingManager()
    {
        auto dsm = p_CreateGLDeferredShadingManager();
        
        if (!dsm->Init(eb, rk, this))
            return nullptr;

        return dsm;
    }

    shared_ptr<IFontFace> RenderingManager::CreateFontFace(const char* name)
    {
        return p_CreateFontFace(eb, rk, this, name);
    }

    shared_ptr<IFPMaterial> RenderingManager::CreateFPMaterial(const char* name, int flags)
    {
        if (usingCoreProfile)
            return ErrorBuffer::SetError2(eb, EX_FEATURE_DISABLED, 2,
                    "desc", "Fixed-pipeline materials are not available in Core Profile mode."
                    ), nullptr;
        else
            return p_CreateFPMaterial(eb, rk, this, name, flags);
    }

    shared_ptr<IGeomBuffer> RenderingManager::CreateGeomBuffer(const char* name)
    {
        return p_CreateGeomBuffer(eb, rk, this, name);
    }

    shared_ptr<zfw::IGraphics> RenderingManager::CreateGraphicsFromTexture(shared_ptr<ITexture> texture)
    {
        const Float2 uv[] = { Float2(0.0f, 0.0f), Float2(1.0f, 1.0f) };

        auto g = p_CreateGLGraphics();
        
        if (!g->InitWithTexture(eb, rk, this, move(texture), uv))
            return nullptr;

        return g;
    }

    shared_ptr<zfw::IGraphics> RenderingManager::CreateGraphicsFromTexture2(shared_ptr<ITexture> texture, const Float2 uv[2])
    {
        auto g = p_CreateGLGraphics();
        
        if (!g->InitWithTexture(eb, rk, this, move(texture), uv))
            return nullptr;

        return g;
    }

    shared_ptr<IMaterial> RenderingManager::CreateMaterial(const char* name, shared_ptr<IShader> program)
    {
        return p_CreateMaterial(eb, rk, this, name,
                std::static_pointer_cast<IGLShaderProgram>(program));
    }

    shared_ptr<IRenderBuffer> RenderingManager::CreateRenderBuffer(const char* name, Int2 size, int flags)
    {
        auto rb = p_CreateGLRenderBuffer();
        
        if (!rb->Init(eb, rk, this, name, size))
            return nullptr;

        if (flags & kRenderBufferColourTexture)
        {
            auto texture = p_CreateTexture(eb, rk, this, name);
            texture->SetContentsUndefined(size, kTextureNoMipmaps, kTextureRGBA8);
            rb->GLAddColourAttachment(move(texture));
        }

        if (flags & kRenderBufferDepthTexture)
        {
            auto texture = p_CreateTexture(eb, rk, this, name);
            texture->SetContentsUndefined(size, kTextureNoMipmaps, kTextureDepth);
            rb->GLSetDepthAttachment(move(texture));
        }

        return rb;
    }

    shared_ptr<IRenderBuffer> RenderingManager::CreateRenderBufferWithTexture(const char* name, shared_ptr<ITexture> texture, int flags)
    {
        auto rb = p_CreateGLRenderBuffer();
        
        if (!rb->Init(eb, rk, this, name, texture->GetSize()))
            return nullptr;

        rb->GLAddColourAttachment(std::static_pointer_cast<IGLTexture>(texture));

        return rb;
    }

    shared_ptr<IResource> RenderingManager::CreateResource(IResourceManager* res, const std::type_index& resourceClass, const char* normparams, int flags)
    {
        if (resourceClass == typeid(IFontFace))
        {
            String path;
            int size, flags;

            if (!gameui::UIThemerUtil::IsFontParams(normparams, path, size, flags))
                return ErrorBuffer::SetError2(eb, EX_INVALID_ARGUMENT, 0), nullptr;

            auto face = p_CreateFontFace(eb, rk, this, normparams);

            if (!face->Open(path, size, flags))
                return nullptr;

            return face;
        }
        else if (resourceClass == typeid(IGraphics))
        {
            auto g = p_CreateGLGraphics();

            if (!g->Init(eb, rk, this, res, normparams, flags))
                return nullptr;

            return g;
        }
        else if (resourceClass == typeid(IMaterial))
        {
            String texture;

            const char *key, *value;

            while (Params::Next(normparams, key, value))
            {
                if (strcmp(key, "texture") == 0)
                    texture = value;
            }

            auto material = p_CreateFPMaterial(eb, rk, this, "", 0);
                
            if (!texture.isEmpty())
            {
                shared_ptr<ITexture> tex = res->GetResource<ITexture>(texture, RESOURCE_REQUIRED, 0);

                if (tex == nullptr)
                    return nullptr;

                material->SetNumTextures(1);
                material->SetTexture(0, move(tex));
            }

            return material->GetMaterial();
        }
        else if (resourceClass == typeid(IShader) || resourceClass == typeid(IGLShaderProgram))
        {
            String path;
            String outputNames;

            const char *key, *value;

            while (Params::Next(normparams, key, value))
            {
                if (strcmp(key, "path") == 0)
                    path = value;
                else if (strcmp(key, "outputNames") == 0)
                    outputNames = value;
            }

            // FIXME: Replace with a nice check
            ZFW_ASSERT(!path.isEmpty())

            auto program = p_CreateShaderProgram(eb, rk, this, path);

            if (!outputNames.isEmpty())
            {
                List<const char*> outputNames_;

                // FIXME: This will break one day
                char* search = const_cast<char*>(outputNames.c_str());

                while (true)
                {
                    char* space = strchr(search, ' ');

                    if (space != nullptr)
                    {
                        *space = 0;
                        outputNames_.add(search);
                        search = space + 1;
                    }
                    else
                    {
                        outputNames_.add(search);
                        break;
                    }
                }

                outputNames_.add(nullptr);

                if (!program->GLCompile(path, outputNames_.c_array()))
                    return nullptr;
            }
            else
            {
                if (!program->GLCompile(path, nullptr))
                    return nullptr;
            }

            return program;
        }
        else if (resourceClass == typeid(ITexture))
        {
            String path;
            RKTextureWrap_t wrapx = kTextureWrapClamp, wrapy = kTextureWrapClamp;

            const char *key, *value;

            while (Params::Next(normparams, key, value))
            {
                if (strcmp(key, "path") == 0)
                    path = value;
                else if (strcmp(key, "wrapx") == 0)
                {
                    if (strcmp(value, "repeat") == 0)
                        wrapx = kTextureWrapRepeat;
                }
                else if (strcmp(key, "wrapy") == 0)
                {
                    if (strcmp(value, "repeat") == 0)
                        wrapy = kTextureWrapRepeat;
                }
            }

            // FIXME: Replace with a nice check
            ZFW_ASSERT(!path.isEmpty())

            Pixmap_t pm;

            if (!rk->GetHost()->LoadBitmap(path, &pm))
                return nullptr;

            auto texture = p_CreateTexture(eb, rk, this, path);

            texture->SetWrapMode(0, wrapx);
            texture->SetWrapMode(1, wrapy);

            PixmapWrapper container(&pm, false);
            if (!texture->SetContentsFromPixmap(&container))
                return nullptr;

            return texture;
        }
        else
        {
            ZFW_ASSERT(resourceClass != resourceClass)
            return nullptr;
        }
    }

    /*ISceneGraph* RenderingManager::CreateSceneGraph(const char* name)
    {
        return p_CreateSceneGraph(eb, rk, this, name);
    }*/

    shared_ptr<ITexture> RenderingManager::CreateTexture(const char* name)
    {
        return p_CreateTexture(eb, rk, this, name);
    }

    shared_ptr<ITextureAtlas> RenderingManager::CreateTextureAtlas2D(const char* name, Int2 size)
    {
        auto texture = p_CreateTexture(eb, rk, this, sprintf_255("%s/texture", name));
        texture->SetContentsUndefined(size, kTextureNoMipmaps, kTextureRGBA8);

        auto pixelAtlas = new PixelAtlas(size, Int2(2, 2), Int2(0, 0), Int2(8, 8));

        auto atlas = p_CreateTextureAtlas(eb, rk, this, name);
        atlas->GLInitWith(move(texture), pixelAtlas);
        return atlas;
    }

    /*void RenderingManager::DrawFilledRectangle(const Float3& pos, const Float2& size, Byte4 colour)
    {
        ImmedVertex* vertices = static_cast<ImmedVertex*>(vertexCache->Alloc(&vertexCacheQuads, 4 * sizeof(ImmedVertex)));

        vertices[0].x = pos.x;
        vertices[0].y = pos.y;
        vertices[0].z = pos.z;
        vertices[0].u = 0.0f;
        vertices[0].v = 0.0f;
        memcpy(&vertices[0].rgba[0], &colour[0], 4);

        vertices[1].x = pos.x;
        vertices[1].y = pos.y + size.y;
        vertices[1].z = pos.z;
        vertices[1].u = 0.0f;
        vertices[1].v = 1.0f;
        memcpy(&vertices[1].rgba[0], &colour[0], 4);

        vertices[2].x = pos.x + size.x;
        vertices[2].y = pos.y + size.y;
        vertices[2].z = pos.z;
        vertices[2].u = 1.0f;
        vertices[2].v = 1.0f;
        memcpy(&vertices[2].rgba[0], &colour[0], 4);

        vertices[3].x = pos.x + size.x;
        vertices[3].y = pos.y;
        vertices[3].z = pos.z;
        vertices[3].u = 1.0f;
        vertices[3].v = 0.0f;
        memcpy(&vertices[3].rgba[0], &colour[0], 4);

        vertexCache->Flush();
    }

    void RenderingManager::DrawLines(IGeomChunk* gc)
    {
        vertexCache->Flush();

        p_DrawChunk(gc, currentMaterialRef, GL_LINES);
    }

    void RenderingManager::DrawQuads(IGeomChunk* gc)
    {
        vertexCache->Flush();

        p_DrawChunk(gc, currentMaterialRef, GL_QUADS);
    }

    void RenderingManager::DrawTriangles(IGeomChunk* gc)
    {
        vertexCache->Flush();

        p_DrawChunk(gc, currentMaterialRef, GL_TRIANGLES);
    }*/

    void RenderingManager::DrawPrimitives(IMaterial* material, RKPrimitiveType_t primitiveType, IGeomChunk* gc)
    {
        if (usingCoreProfile)
            zombie_assert(primitiveType != RK_QUADS_DEPRECATED);

        vertexCache.cache->Flush();

        p_DrawChunk(this, gc, static_cast<IGLMaterial*>(material), primitiveTypeToGLMode[primitiveType]);
    }

    void RenderingManager::EndFrame(int ticksElapsed)
    {
        vertexCache.cache->Flush();

        this->CheckErrors(li_functionName);

        /*if (profileFrame == frameNumber)
        {
            profiler->LeaveSection(prof_drawframe);
            profiler->EnterSection(prof_SwapBuffers);
        }*/

        // TODO: which would be the correct order for these?
        // (consider how delta calculation works)
        glFlush();

        /*uint8_t* cap_rgb;
        if (cap != nullptr && cap->OfferFrameBGRFlipped(0, payload->ticksElapsed, &cap_rgb) > 0)
        {
            glReadPixels(0, 0, gl.renderWindow.x, gl.renderWindow.y, GL_BGR, GL_UNSIGNED_BYTE, cap_rgb);
            cap->OnFrameReady(0);
        }*/

        rk->GetWindowManagerBackend()->Present();

        /*if (profileFrame == frameNumber)
        {
            profiler->LeaveSection(prof_SwapBuffers);
            profiler->PrintProfile();

            host->Printk(true, "Rendering Kit Profile: %u mat, %u tex, %u vbo, %u draw, %u sceneB",
                    gl.numMaterialSetups, gl.numTextureBinds, gl.numVboBinds, gl.numDrawCalls, gl.sceneBytesSent);
        }*/

        //frameNumber++;
    }

    zfw::IResourceManager* RenderingManager::GetSharedResourceManager()
    {
        if (!sharedResourceManager)
        {
            sharedResourceManager.reset(rk->GetSys()->CreateResourceManager("RenderingManager::sharedResourceManager"));
            this->RegisterResourceProviders(sharedResourceManager.get());
        }

        return sharedResourceManager.get();
    }

    void RenderingManager::GetViewportPosAndSize(Int2* viewportPos_out, Int2* viewportSize_out)
    {
        *viewportPos_out = viewportPos;
        *viewportSize_out = viewportSize;
    }

    void RenderingManager::OnVertexCacheFlush(IGLVertexCache* vc, size_t bytesUsed)
    {
        this->CheckErrors(li_functionName);

        GLStateTracker::BindArrayBuffer(vc->GetVBO());

        SetupMaterialAndVertexFormat(vertexCache.material, vertexCache.options, vertexCache.vertexFormat, vc->GetVBO());
        this->CheckErrors("this->SetupMaterialAndVertexFormat");

        // Ignore bytesUsed, we're tracking this ourselves (don't have to ask VertexFormat this way)
        glDrawArrays(primitiveTypeToGLMode[vertexCache.primitiveType], 0, vertexCache.numVertices);
        this->CheckErrors("glDrawArrays");

        CleanupMaterialAndVertexFormat();
        this->CheckErrors("this->CleanupMaterialAndVertexFormat");

        vertexCache.material = nullptr;
        vertexCache.vertexFormat = nullptr;
        vertexCache.numVertices = 0;
    }

    void RenderingManager::OnWindowResized(Int2 newSize)
    {
        windowSize = newSize;
        framebufferSize = windowSize;
    }

    void RenderingManager::p_SetRenderBuffer(IGLRenderBuffer* rb)
    {
        VertexCacheFlush();

        if (rb != nullptr)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, rb->GLGetFBO());
            framebufferSize = rb->GetViewportSize();
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            framebufferSize = windowSize;
        }

        this->CheckErrors(li_functionName);
        SetViewportPosAndSize(Int2(), framebufferSize);
    }

    void RenderingManager::PopRenderBuffer()
    {
        renderBufferStack.pop();

        auto rb = (!renderBufferStack.isEmpty()) ? renderBufferStack.top() : nullptr;
        p_SetRenderBuffer(rb);
    }

    void RenderingManager::PushRenderBuffer(IRenderBuffer* rb_in)
    {
        auto rb = static_cast<IGLRenderBuffer*>(rb_in);

        renderBufferStack.push(rb);
        p_SetRenderBuffer(rb);
    }

    bool RenderingManager::ReadPixel(Int2 posInFB, Byte4* colour_outOrNull, float* depth_outOrNull)
    {
        if (posInFB.x < 0 || posInFB.y < 0 || posInFB.x >= framebufferSize.x || posInFB.y >= framebufferSize.y)
            return false;

        if (depth_outOrNull != nullptr)
        {
            float depth = 1.0f;
            glReadPixels(posInFB.x, framebufferSize.y - posInFB.y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

            *depth_outOrNull = depth * 2.0f - 1.0f;
        }

        if (colour_outOrNull != nullptr)
        {
            uint8_t rgba[4] = {};
            glReadPixels(posInFB.x, framebufferSize.y - posInFB.y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &rgba);

            memcpy(colour_outOrNull, rgba, 4);
        }

        return true;
    }

    void RenderingManager::RegisterResourceProviders(zfw::IResourceManager* res)
    {
        static const std::type_index resourceClasses[] = {
            typeid(IFontFace), typeid(IGraphics), typeid(IMaterial), typeid(IShader), typeid(ITexture),
            typeid(IGLShaderProgram)
        };

        res->RegisterResourceProvider(resourceClasses, lengthof(resourceClasses), this, 0);
    }

    void RenderingManager::SetCamera(ICamera* camera)
    {
        VertexCacheFlush();

        static_cast<IGLCamera*>(camera)->GLSetUpMatrices(viewportSize, projectionCurrent, modelViewCurrent);
    }

    void RenderingManager::SetClearColour(const Float4& colour)
    {
        glClearColor(colour.r, colour.g, colour.b, colour.a);
    }

    /*void RenderingManager::SetMaterial(IMaterial* materialRef)
    {
        vertexCache->Flush();

        zfw::Release(currentMaterialRef);
        currentMaterialRef = static_cast<IGLMaterial*>(materialRef);
    }*/

    void RenderingManager::SetMaterialOverride(IMaterial* materialOrNull)
    {
        this->materialOverride = static_cast<IGLMaterial*>(materialOrNull);
    }

    void RenderingManager::SetProjection(const glm::mat4x4& projection)
    {
        //glMatrixMode(GL_PROJECTION);
        //glLoadMatrixf(&projection[0][0]);

        // TODO: Not quite happy about this

        static glm::mat4x4 projection_;

        projection_ = projection;
        this->projectionCurrent = &projection_;
    }

    void RenderingManager::SetProjectionOrthoScreenSpace(float nearZ, float farZ)
    {
        cam->SetClippingDist(nearZ, farZ);
        cam->SetOrthoScreenSpace();
        SetCamera(cam.get());
    }

    void RenderingManager::SetRenderState(RKRenderStateEnum_t state, int value)
    {
        VertexCacheFlush();

        switch (state)
        {
            case RK_DEPTH_TEST: GLStateTracker::SetState(ST_GL_DEPTH_TEST, value != 0); break;
        }
    }

    void RenderingManager::SetViewportPosAndSize(Int2 viewportPos, Int2 viewportSize)
    {
        VertexCacheFlush();

        this->viewportPos = viewportPos;
        this->viewportSize = viewportSize;

        glViewport(viewportPos.x, framebufferSize.y - viewportPos.y - viewportSize.y, viewportSize.x, viewportSize.y);
    }

    bool RenderingManager::Startup()
    {
        rk->GetSys()->Printf(kLogInfo, "Rendering Kit: %s | %s | %s", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

        /*if (GLEW_ARB_debug_output)
        {
            glDebugMessageCallbackARB(glDebugProc, nullptr);
            glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 1, GL_DEBUG_SEVERITY_MEDIUM_ARB, 12, "Hello World!");
        }
        else if (GLEW_AMD_debug_output)
        {
            glDebugMessageCallbackAMD(glDebugProcAMD, nullptr);
            glDebugMessageEnableAMD(0, 0, 0, nullptr, GL_TRUE);

            glDebugMessageInsertAMD(GL_DEBUG_CATEGORY_APPLICATION_AMD, GL_DEBUG_SEVERITY_MEDIUM_AMD, 1, 12, "Hello World!");
        }*/

        windowSize = rk->GetWindowManagerBackend()->GetWindowSize();

        GLStateTracker::Init();
        CheckErrors(li_functionName);
        bool haveVAOs = (GLEW_ARB_vertex_array_object > 0);

        GLStateTracker::SetState(ST_GL_BLEND, true);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        CheckErrors("glBlendFunc");

        glEnable(GL_CULL_FACE);
        CheckErrors("glEnable(GL_CULL_FACE)");

        // Init private resources
        vertexCache.cache = unique_ptr<IGLVertexCache>(p_CreateVertexCache(eb, rk, this, 256 * 1024));
        vertexCache.numVertices = 0;
        //immedVertexFormat = CompileVertexFormat(nullptr, 24, immedVertexAttribs, false);

        framebufferSize = windowSize;
        SetViewportPosAndSize(Int2(), framebufferSize);
        CheckErrors(li_functionName);
        if (!usingCoreProfile)
        {
            glMatrixMode(GL_TEXTURE);
            glTranslatef(0.0f, 1.0f, 0.0f);
            glScalef(1.0f, -1.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);
        }

        CheckErrors(li_functionName);
        return true;
    }

    const char* RenderingManager::TryGetResourceClassName(const std::type_index& resourceClass)
    {
        if (resourceClass == typeid(::RenderingKit::IFontFace))
            return "RenderingKit::IFontFace";
        else if (resourceClass == typeid(::RenderingKit::IGraphics))
            return "RenderingKit::IGraphics";
        else if (resourceClass == typeid(::RenderingKit::IMaterial))
            return "RenderingKit::IMaterial";
        else if (resourceClass == typeid(::RenderingKit::IShader))
            return "RenderingKit::IShader";
        else if (resourceClass == typeid(::RenderingKit::ITexture))
            return "RenderingKit::ITexture";
        else
            return nullptr;
    }

    void RenderingManager::SetupMaterialAndVertexFormat(IGLMaterial* material, const MaterialSetupOptions& options,
            GLVertexFormat* vertexFormat, GLuint vbo)
    {
        if (materialOverride != nullptr)
            material = materialOverride;

        int fpMaterialFlags;
        IFPMaterial* fpMaterial = dynamic_cast<IFPMaterial*>(material);  // FIXME: needs to be optimized ASAP

        if (fpMaterial != nullptr)
            fpMaterialFlags = fpMaterial->GetFlags();
        else
            fpMaterialFlags = 0;

        // kFPMaterialIgnoreVertexColour

        material->GLSetup(options, *projectionCurrent, *modelViewCurrent);
        vertexFormat->Setup(vbo, fpMaterialFlags);

        currentVertexFormat = vertexFormat;
    }

    void* RenderingManager::VertexCacheAlloc(IVertexFormat* vertexFormat, IMaterial* material,
            RKPrimitiveType_t primitiveType, size_t numVertices)
    {
        if (vertexCache.vertexFormat != vertexFormat
                || vertexCache.material != material
                || vertexCache.options.type != MaterialSetupOptions::kNone
                || vertexCache.primitiveType != primitiveType)
        {
            vertexCache.cache->Flush();

            vertexCache.vertexFormat = static_cast<GLVertexFormat*>(vertexFormat);
            vertexCache.material = static_cast<IGLMaterial*>(material);
            vertexCache.options.type = MaterialSetupOptions::kNone;
            vertexCache.primitiveType = primitiveType;
        }

        void* p = vertexCache.cache->Alloc(this, numVertices * static_cast<GLVertexFormat*>(vertexFormat)
                ->GetVertexSizeNonvirtual());

        vertexCache.numVertices += numVertices;
        return p;
    }

    void* RenderingManager::VertexCacheAlloc(IVertexFormat* vertexFormat, IMaterial* material,
            const MaterialSetupOptions& options, RKPrimitiveType_t primitiveType, size_t numVertices)
    {
        if (vertexCache.vertexFormat != vertexFormat
                || vertexCache.material != material
                || !(vertexCache.options == options)
                || vertexCache.primitiveType != primitiveType)
        {
            vertexCache.cache->Flush();

            vertexCache.vertexFormat = static_cast<GLVertexFormat*>(vertexFormat);
            vertexCache.material = static_cast<IGLMaterial*>(material);
            vertexCache.options = options;
            vertexCache.primitiveType = primitiveType;
        }

        void* p = vertexCache.cache->Alloc(this, numVertices * static_cast<GLVertexFormat*>(vertexFormat)
                ->GetVertexSizeNonvirtual());

        vertexCache.numVertices += numVertices;
        return p;
    }

    void RenderingManager::VertexCacheFlush()
    {
        this->CheckErrors(li_functionName);

        // PROTIP: this will call our OnVertexCacheFlush
        vertexCache.cache->Flush();

        this->CheckErrors("vertexCache.cache->Flush()");
    }
}