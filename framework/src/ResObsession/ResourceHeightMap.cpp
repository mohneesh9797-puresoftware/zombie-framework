#include <Res/ResourceHeightMap.hpp>

#include <framework/utility/params.hpp>
#include <framework/utility/pixmap.hpp>

namespace Obs::Res {

ResourceHeightMap::ResourceHeightMap(ResourceParamSet const& params) {
    auto normparams = params.AsParamsString();
    const char *key, *value;

    while (zfw::Params::Next(normparams, key, value)) {
        if (strcmp(key, "path") == 0) {
            path = value;
        }
    }

    zombie_assert(!path.empty());
}

Data::HeightMap const& ResourceHeightMap::Get() {
    zombie_assert(state == REALIZED);
    return *heightMap;
}

bool ResourceHeightMap::Preload(IResourceManager* resMgr) {
    zfw::Pixmap_t pm;

    if (!zfw::Pixmap::LoadFromFile(resMgr->GetEngine(), pm, path.c_str())) {
        return false;
    }

    heightMap = std::make_unique<Obs::Data::HeightMap>(pm);
    return true;
}

}
