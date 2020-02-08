#ifndef OBS_RES_RESOURCEHEIGHTMAP_HPP
#define OBS_RES_RESOURCEHEIGHTMAP_HPP

#include "Resource.hpp"
#include "ResourceManager.hpp"

#include <Data/HeightMap.hpp>

namespace Obs::Res {

class ResourceHeightMap : public IResource {
public:
    explicit ResourceHeightMap(ResourceParamSet const& params);

    Data::HeightMap const& Get();

    virtual void* Cast(const zfw::TypeID& resourceClass) final override { return DefaultCast(this, resourceClass); }

    virtual State_t GetState() const final override { return this->state; }

    virtual bool StateTransitionTo(State_t targetState, IResourceManager* resMgr) final override
    {
        return DefaultStateTransitionTo(this, targetState, resMgr);
    }

private:
    bool BindDependencies(IResourceManager* resMgr) { return true; }
    bool Preload(IResourceManager* resMgr);
    void Unload() { heightMap.reset(); }
    bool Realize(IResourceManager* resMgr) { return true; }
    void Unrealize() {}

    State_t state = CREATED;

    std::string path;
    std::unique_ptr<Data::HeightMap> heightMap;

    friend IResource;
};

}

#endif
