#pragma once

#include <framework/engine.hpp>
#include <framework/resourcemanager2.hpp>
#include <framework/utility/essentials.hpp>

#include <RenderingKit/RenderingKit.hpp>

#include <vector>

namespace Client {

class Ms3dModel : public zfw::IResource2 {
public:
    Ms3dModel(std::string path, zfw::IEngine& engine, RenderingKit::IRenderingManager& rm)
        : path(path)
        , engine(engine)
        , rm(rm) {}

    void* Cast(const zfw::TypeID& resourceClass) override { return DefaultCast(this, resourceClass); }
    State_t GetState() const override { return this->state; }
    bool StateTransitionTo(State_t targetState, zfw::IResourceManager2* resMgr) override {
        return DefaultStateTransitionTo(this, targetState, resMgr);
    }

    void Draw(glm::mat4x4 const& transform);

private:
    struct Vertex {
        float pos[3];
        float normal[3];
        float uv[2];
        float rgba[4];
    };

    struct Mesh {
        // preloaded data
        std::vector<Vertex> vertices;
        int materialIndex;

        RenderingKit::IMaterial* material = nullptr;

        // realized data
        std::unique_ptr<RenderingKit::IGeomChunk> gc;
    };

    struct MaterialDesc {
        std::string texturePath;
    };

    bool BindDependencies(zfw::IResourceManager2* resMgr) { return true; }

    bool Preload(zfw::IResourceManager2* resMgr);
    void Unload() {
        meshes.clear();
        materialDescs.clear();
    }

    bool Realize(zfw::IResourceManager2* resMgr);
    void Unrealize();

    State_t state = State_t::CREATED;
    std::string path;

    // preloaded data
    std::vector<Mesh> meshes;
    std::vector<MaterialDesc> materialDescs;

    // realized data
    std::shared_ptr<RenderingKit::IGeomBuffer> gb;

    zfw::IEngine& engine;
    RenderingKit::IRenderingManager& rm;

    friend class zfw::IResource2;
};

class Ms3dModelResourceProvider : public zfw::IResourceProvider2 {
public:
    Ms3dModelResourceProvider(zfw::IEngine& engine, RenderingKit::IRenderingManager& rm)
        : engine(engine)
        , rm(rm) {}

    zfw::IResource2* CreateResource(
        zfw::IResourceManager2* res, const zfw::TypeID& resourceClass, const char* recipe, int flags) override;

    void RegisterWith(zfw::IResourceManager2& resMgr);

private:
    zfw::IEngine& engine;
    RenderingKit::IRenderingManager& rm;
};

}
