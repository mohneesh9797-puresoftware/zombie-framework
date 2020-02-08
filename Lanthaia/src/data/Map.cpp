
#include "Map.hpp"

#include <Res/ResourceHeightMap.hpp>
#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/componenttype.hpp>
#include <framework/engine.hpp>
#include <framework/entityworld2.hpp>

#include <littl/File.hpp>

#include <framework/resourcemanager2.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Client {

using glm::vec3;
using std::make_unique;
using std::unique_ptr;

using Obs::Res::ResourceHeightMap;

extern zfw::IResourceManager2* globalResMgr;

Sector::Sector(zfw::IEngine& engine, zfw::IEntityWorld2& world, Map* map, unsigned sx, unsigned sy)
    : map(map)
    , sx(sx)
    , sy(sy) {
    li::String fileName = (li::String) "tolcl/area/" + sx + "+" + sy + ".sect";

    std::unique_ptr<li::InputStream> sectorFile { engine.OpenInput(fileName) };

    if (!sectorFile) {
        engine.Printf(zfw::kLogWarning, "Sector file '%s' not found", fileName.c_str());
        return;
    }

    for (;;) {
        uint8_t entType;
        if (!sectorFile->readLE(&entType)) {
            break;
        }

        if (entType == 0)
            break;
        else if (entType == 1) {
            // WorldMesh, v1

            uint16_t wmid;
            sectorFile->readLE(&wmid);

            meshIds.add(wmid);
            map->addReference(engine, world, wmid);
        }
        else if (entType == 2) {
            // WorldObj, v1

            li::String name = sectorFile->readString();
            float x, y, o;
            sectorFile->readLE(&x);
            sectorFile->readLE(&y);
            sectorFile->readLE(&o);

            addWorldObj(engine, world, name, x + sx * 200.0f, y + sy * 200.0f, 0.0, o * zfw::f_pi / 180.0f);
        }
        else
            printf("ERROR Sector::Sector() : unknown ent-type %02X\n", entType);
    }
}

Sector::~Sector() {
    for (auto& meshId : meshIds)
        map->removeReference(meshId);
}

void Sector::addWorldObj(
    zfw::IEngine& engine, zfw::IEntityWorld2& world, const li::String& name, float x, float y, float z, float o) {
    std::unique_ptr<li::InputStream> objectInfo { engine.OpenInput((li::String) "tolcl/world/" + name + ".obj") };

    if (!objectInfo || objectInfo->eof()) {
        printf("ERROR Sector::addWorldObj() : load WorldObj (%s) failed.\n", name.c_str());
        return;
    }

    objectInfo->readLine();

    li::String type = objectInfo->readLine();

    if (type == "light") {
    }
    else if (type == "model") {
        li::String source = objectInfo->readLine();

        z = map->getHeightAt(x, y);

        engine.Printf(zfw::kLogDebugInfo, "addWorldObj %s at [%g %g %g] orientation %g", source.c_str(), x, y, z, o);

        auto entity = world.CreateEntity();
        entity.SetComponent(zfw::Model3D { source.c_str() });
        entity.SetComponent(zfw::Position { { x, y, z }, glm::angleAxis(o, vec3 { 0.0f, 0.0f, 1.0f }) });
    }
}

Map::Map(float xc, float yc, zfw::IEngine& engine, zfw::IEntityWorld2& world)
    : csx(-1)
    , csy(-1) {
    memset(sectors, 0, sizeof(sectors));

    csx = (unsigned)floor(xc / 200.f);
    csy = (unsigned)floor(yc / 200.f);

    for (unsigned x = 1; x < 4; x++)
        for (unsigned y = 1; y < 4; y++)
            load(engine, world, x, y);

    // TODO: remove one time
    // world->enter();

    // Start the world sorter
    // Will block for some time as a call to map->lock() occurs from GameScene::render() shortly after calling this
    // constructor Whatever.
    //        world->start();
}

void Map::addReference(zfw::IEngine& engine, zfw::IEntityWorld2& world, unsigned wmid) {
    printf("Map::addReference( %u )\n", wmid);

    for (auto& terrain : terrains)
        if (terrain.wmid == wmid) {
            printf(" -- is a known terrain -> ignoring all other params\n");
            terrain.numRefs++;
            return;
        }

    printf(" -- not found, loading %s\n", ((li::String) "tolcl/world/" + wmid + ".mesh").c_str());

    std::unique_ptr<li::InputStream> meshInfo { engine.OpenInput((li::String) "tolcl/world/" + wmid + ".mesh") };

    if (!meshInfo || meshInfo->eof()) {
        printf("ERROR Map::addReference() : load WorldMesh (#%u) failed.\n", wmid);
        return;
    }

    meshInfo->readLine();

    li::String type = meshInfo->readLine();

    if (type == "heightmap") {
        TerrainHeightMap terrain;

        terrain.path = meshInfo->readLine();
        terrain.texture = meshInfo->readLine();

        auto x = meshInfo->readLine().toFloat();
        auto y = meshInfo->readLine().toFloat();
        auto z = meshInfo->readLine().toFloat();
        terrain.scale.x = meshInfo->readLine().toFloat();
        terrain.scale.y = meshInfo->readLine().toFloat();
        terrain.scale.z = meshInfo->readLine().toFloat();

        auto heightMap = globalResMgr->GetResourceByPath<ResourceHeightMap>(
            terrain.path.c_str(), zfw::IResourceManager2::kResourceRequired);
        zombie_assert(heightMap);

        auto entity = world.CreateEntity();
        auto& ref_terrain = entity.SetComponent(std::move(terrain));
        auto& ref_pos = entity.SetComponent(zfw::Position { { x, y, z }, glm::quat(0.0f, 0.0f, 0.0f, 1.0f) });

        terrains.emplace_back(TerrainRef { wmid, 1, entity, ref_pos, ref_terrain, *heightMap });
    }
}

float Map::getHeightAt(float x, float y) {
    for (auto& terrain : terrains) {
        const TerrainRef& curr = terrain;

        auto& heightMap = curr.heightMap.Get();
        auto scale = curr.terrain.scale;

        if (x >= curr.pos.pos.x && y >= curr.pos.pos.y && x < curr.pos.pos.x + scale.x
            && y < curr.pos.pos.y + scale.y) {
            return curr.pos.pos.z
                + heightMap.SampleBilinear({ (x - curr.pos.pos.x) / scale.x, (y - curr.pos.pos.y) / scale.y })
                * scale.z;
        }
    }

    return 0.0f;
}

Sector* Map::getSectorAt(float x, float y) {
    int mx = (unsigned)floor(x / 200.f) - csx + 2;
    int my = (unsigned)floor(y / 200.f) - csy + 2;

    printf("Spawning in local sector %i, %i\n", mx, my);

    if (mx > 0 && my > 0 && mx < 5 && my < 5)
        return sectors[mx][my];
    else
        return 0;
}

void Map::load(zfw::IEngine& engine, zfw::IEntityWorld2& world, unsigned mx, unsigned my) {
    zombie_assert(mx < 5);
    zombie_assert(my < 5);

    if (sectors[mx][my]) {
        delete sectors[mx][my];
        sectors[mx][my] = 0;
    }

    // cs* + m* - 2 (world sector coords) must be > 0
    if (csx + mx >= 2 && csy + my >= 2) {
        printf("Loading sector (%u %u) at [%u %u]\n", csx - 2 + mx, csy - 2 + my, mx, my);
        sectors[mx][my] = new Sector(engine, world, this, csx - 2 + mx, csy - 2 + my);
    }
}

void Map::moveCenter(zfw::IEngine& engine, zfw::IEntityWorld2& world, float xc, float yc) {
    bool sectChange = false;

    int newCsx = (unsigned)floor(xc / 200.0f);
    int newCsy = (unsigned)floor(yc / 200.0f);

    if (csx != newCsx) {
        shiftRight(csx - newCsx);
        csx = newCsx;
        sectChange = true;
    }

    if (csy != newCsy) {
        shiftDown(csy - newCsy);
        csy = newCsy;
        sectChange = true;
    }

    if (sectChange) {
        for (unsigned x = 1; x < 4; x++)
            for (unsigned y = 1; y < 4; y++)
                if (!sectors[x][y])
                    load(engine, world, x, y);
    }
}

void Map::removeReference(unsigned wmid) {
    zombie_assert(false);

    // Code below is correct, but doesn't compile
    //    for (size_t i = 0; i < terrains.size(); i++)
    //        if (terrains[i].wmid == wmid) {
    //            if (--terrains[i].numRefs == 0) {
    //                // terrains[i].model->release();
    //                terrains[i].entity.Destroy();
    //                terrains.erase(terrains.begin() + i);
    //            }
    //
    //            return;
    //        }
}

void Map::shiftDown(int count) {
    while (count > 0) {
        for (unsigned x = 0; x < 5; x++) {
            for (unsigned y = 4; y >= 1; y--)
                sectors[x][y] = sectors[x][y - 1];

            sectors[x][0] = 0;
        }

        count--;
    }

    while (count < 0) {
        for (unsigned x = 0; x < 5; x++) {
            for (unsigned y = 1; y < 5; y++)
                sectors[x][y - 1] = sectors[x][y];

            sectors[x][4] = 0;
        }

        count++;
    }
}

void Map::shiftRight(int count) {
    while (count > 0) {
        for (unsigned y = 0; y < 5; y++) {
            for (unsigned x = 4; x >= 1; x--)
                sectors[x][y] = sectors[x - 1][y];

            sectors[0][y] = 0;
        }

        count--;
    }

    while (count < 0) {
        for (unsigned y = 0; y < 5; y++) {
            for (unsigned x = 1; x < 5; x++)
                sectors[x - 1][y] = sectors[x][y];

            sectors[4][y] = 0;
        }

        count++;
    }
}

}
