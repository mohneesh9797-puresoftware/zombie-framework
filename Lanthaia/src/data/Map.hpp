#ifndef LANTHAIA_MAP_HPP
#define LANTHAIA_MAP_HPP

#include <Ecs/TerrainHeightMap.hpp>
#include <Res/ResourceHeightMap.hpp>

#include <framework/entityworld2.hpp>

#include <littl/String.hpp>

namespace Client {

using Obs::Ecs::TerrainHeightMap;
using Obs::Res::ResourceHeightMap;

struct Map;

class Sector {
    Map* map;
    unsigned sx, sy;

    li::List<unsigned> meshIds;

public:
    Sector(zfw::IEngine& engine, zfw::IEntityWorld2& world, Map* map, unsigned sx, unsigned sy);
    ~Sector();

    void addWorldObj(
        zfw::IEngine& engine, zfw::IEntityWorld2& world, const li::String& name, float x, float y, float z, float o);
    void save();
    void saveAs(const li::String& fileName);
};

struct Map {
    Sector* sectors[5][5];
    int csx, csy;

    friend class Sector;

    // TODO: need to ensure that deleting the corresponding entity (which owns heightMap) also deletes this
    // OR rething ownership of HeightMap
    struct TerrainRef {
        unsigned int wmid;
        int numRefs;

        zfw::EntityHandle const entity;
        zfw::Position const& pos;
        TerrainHeightMap const& terrain;
        ResourceHeightMap& heightMap;
    };

    std::vector<TerrainRef> terrains;

public:
    Map(float xc, float yc, zfw::IEngine& engine, zfw::IEntityWorld2& world);

    void addReference(zfw::IEngine& engine, zfw::IEntityWorld2& world, unsigned wmid);
    float getHeightAt(float x, float y);
    Sector* getSectorAt(float x, float y);
    void load(zfw::IEngine& engine, zfw::IEntityWorld2& world, unsigned mx, unsigned my);
    void moveCenter(zfw::IEngine& engine, zfw::IEntityWorld2& world, float xc, float yc);
    void shiftDown(int count);
    void shiftRight(int count);
    void removeReference(unsigned wmid);

private:
};

}

#endif