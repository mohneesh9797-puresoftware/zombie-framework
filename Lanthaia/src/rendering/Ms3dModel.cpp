/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "Ms3dModel.hpp"

#include <framework/engine.hpp>
#include <framework/resourcemanager2.hpp>
#include <framework/utility/essentials.hpp>

#include <RenderingKit/RenderingKit.hpp>

#include <littl/Stream.hpp>

#include <vector>

#define MAX_VERTICES    8192
#define MAX_TRIANGLES   16384
#define MAX_GROUPS      128
#define MAX_MATERIALS   128
#define MAX_JOINTS      128
#define MAX_KEYFRAMES   216

#define SELECTED        1
#define HIDDEN          2
#define SELECTED2       4
#define DIRTY           8

#pragma pack(1)

namespace Client {
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

struct ms3d_header_t {
    char id[10];
    int version;
}
#ifdef __GNUC__
__attribute((packed))
#endif
;

struct ms3d_vertex_t {
    unsigned char flags;
    float vertex[3];
    char boneId;
    unsigned char referenceCount;
}
#ifdef __GNUC__
__attribute((packed))
#endif
;

struct ms3d_triangle_t {
    unsigned short flags;
    unsigned short vertexIndices[3];
    float vertexNormals[3][3];
    float s[3];
    float t[3];
    unsigned char smoothingGroup;
    unsigned char groupIndex;
}
#ifdef __GNUC__
__attribute((packed))
#endif
;

struct ms3d_group_prologue_t {
    unsigned char flags;
    char name[32];
    unsigned short numTriangles;
}
#ifdef __GNUC__
__attribute((packed))
#endif
;

struct ms3d_material_t {
    char name[32];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emissive[4];
    float shininess;
    float transparency;
    char mode;
    char texture[128];
    char alphamap[128];
}
#ifdef __GNUC__
__attribute((packed))
#endif
;

void Ms3dModel::Draw(glm::mat4x4 const& transform) {
    for (auto& mesh : meshes) {
        rm.DrawPrimitivesTransformed(mesh.material, RenderingKit::RK_TRIANGLES, mesh.gc.get(), transform);
    }
}

bool Ms3dModel::Preload(zfw::IResourceManager2* resMgr) {
    zombie_assert(state == BOUND);

    unique_ptr<li::InputStream> input(engine.OpenInput(this->path.c_str()));

    if (!input)
        return false;

    //        List<IMaterial*> materials;
    //        List<int> materialIndices;

    ms3d_header_t header;
    vector<ms3d_vertex_t> vertices;
    vector<ms3d_triangle_t> polygons;

    uint16_t numVertices, numPolygons, numMeshes, numMaterials;

    zombie_assert(input->read(&header, sizeof(header)) == sizeof(header));

    zombie_assert(header.version == 4);

    input->readLE(&numVertices);

    while (numVertices--) {
        ms3d_vertex_t v;
        input->read(&v, sizeof(v));
        vertices.push_back(v);
    }

    input->readLE(&numPolygons);

    while (numPolygons--) {
        ms3d_triangle_t v;
        input->read(&v, sizeof(v));
        polygons.push_back(v);
    }

    input->readLE(&numMeshes);

    for (unsigned i = 0; i < numMeshes; i++) {
        ms3d_group_prologue_t meshInfo;
        input->read(&meshInfo, sizeof(meshInfo));

        auto& mesh = meshes.emplace_back();

        for (unsigned j = 0; j < meshInfo.numTriangles; j++) {
            uint16_t polyIdx;
            input->readLE(&polyIdx);

            for (int faceVtx = 0; faceVtx < 3; faceVtx++) {
                Vertex v;

                // TODO: justify axis swap

                v.pos[0] = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[0];
                v.pos[1] = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[2];
                v.pos[2] = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[1];

                v.normal[0] = polygons[polyIdx].vertexNormals[faceVtx][0];
                v.normal[1] = polygons[polyIdx].vertexNormals[faceVtx][2];
                v.normal[2] = polygons[polyIdx].vertexNormals[faceVtx][1];

                v.uv[0] = polygons[polyIdx].s[faceVtx];
                v.uv[1] = polygons[polyIdx].t[faceVtx];

                v.rgba[0] = 1.0f;
                v.rgba[1] = 1.0f;
                v.rgba[2] = 1.0f;
                v.rgba[3] = 1.0f;

                mesh.vertices.push_back(v);
            }
        }

        int8_t materialIndex;
        input->readLE(&materialIndex);
        mesh.materialIndex = materialIndex;
    }

    input->readLE(&numMaterials);

    for (unsigned i = 0; i < numMaterials; i++) {
        ms3d_material_t matInfo;
        input->read(&matInfo, sizeof(matInfo));

        MaterialDesc desc {};

        //            properties.colour = Colour( 1.0f, 1.0f, 1.0f, matInfo.transparency );

        if (matInfo.texture[0] != 0) {
            desc.texturePath = matInfo.texture;
        }
        //
        //            if ( resMgr->getLoadFlag( LoadFlag::useDynamicLighting ) != 0 )
        //            {
        //                properties.dynamicLighting = true;
        //                properties.dynamicLightingResponse.ambient = Colour( matInfo.ambient );
        //                properties.dynamicLightingResponse.diffuse = Colour( matInfo.diffuse );
        //                properties.dynamicLightingResponse.specular = Colour( matInfo.specular );
        //                properties.dynamicLightingResponse.emissive = Colour( matInfo.emissive );
        //                properties.dynamicLightingResponse.shininess = matInfo.shininess;
        //            }
        //
        //            if ( resMgr->getLoadFlag( LoadFlag::useShadowMapping ) == 1 )
        //                properties.receivesShadows = true;
        //
        //            materials.add( driver->createMaterial( matInfo.name, &properties, finalized ) );

        materialDescs.push_back(desc);
    }

    for (auto& mesh : meshes) {
        zombie_assert(mesh.materialIndex >= 0 && (unsigned)mesh.materialIndex < materialDescs.size());

        if (materialDescs[mesh.materialIndex].texturePath.empty()) {
            mesh.material = resMgr->GetResourceByPath<RenderingKit::IMaterial>(
                "shader=path=RenderingKit/basic", zfw::IResourceManager2::kResourceRequired);
        }
        else {
            char* materialRecipe
                = zfw::Params::BuildAlloc(2, "shader", "path=RenderingKit/basicTextured", "texture:tex",
                    ("path=tolcl/tex/" + materialDescs[mesh.materialIndex].texturePath + ",wrapx=repeat,wrapy=repeat")
                        .c_str());

            mesh.material = resMgr->GetResource<RenderingKit::IMaterial>(
                materialRecipe, zfw::IResourceManager2::kResourceRequired);
            free(materialRecipe);
        }

        if (!mesh.material) {
            return false;
        }
    }

    return true;
}

bool Ms3dModel::Realize(zfw::IResourceManager2* resMgr) {
    zombie_assert(state == PRELOADED);

    static const RenderingKit::VertexAttrib_t vertexAttribs[] = {
        { "in_Position", 0, RenderingKit::RK_ATTRIB_FLOAT_3 },
        { "in_Normal", 12, RenderingKit::RK_ATTRIB_FLOAT_3 },
        { "in_UV", 24, RenderingKit::RK_ATTRIB_FLOAT_2 },
        { "in_Color", 32, RenderingKit::RK_ATTRIB_FLOAT_4 },
    };

    static void* vfiCache = nullptr;

    static const RenderingKit::VertexFormatInfo vfi { /*.vertexSizeInBytes =*/sizeof(Vertex),
        /*.attribs =*/vertexAttribs,
        /*.numAttribs =*/std::size(vertexAttribs),
        /*.cache =*/&vfiCache };

    gb = rm.CreateGeomBuffer(path.c_str());

    for (auto& mesh : meshes) {
        mesh.gc = gb->AllocVertices(vfi, mesh.vertices.size(), 0);
        mesh.gc->UpdateVertices(0, &mesh.vertices[0], mesh.vertices.size() * sizeof(Vertex));
    }

    return true;
}

void Ms3dModel::Unrealize() {
    for (auto& mesh : meshes) {
        mesh.gc.reset();
    }

    gb.reset();
}

zfw::IResource2* Ms3dModelResourceProvider::CreateResource(
    zfw::IResourceManager2* res, const zfw::TypeID& resourceClass, const char* recipe, int flags) {
    if (resourceClass == typeid(Ms3dModel)) {
        std::string path;

        const char *key, *value;

        while (zfw::Params::Next(recipe, key, value)) {
            if (strcmp(key, "path") == 0)
                path = value;
        }

        zombie_assert(!path.empty());

        return new Ms3dModel(path.c_str(), engine, rm);
    }
    else {
        zombie_assert(resourceClass != resourceClass);
        return nullptr;
    }
}

void Ms3dModelResourceProvider::RegisterWith(zfw::IResourceManager2& resMgr) {
    static const std::type_index resourceClasses[] = {
        typeid(Ms3dModel),
    };

    resMgr.RegisterResourceProvider(resourceClasses, std::size(resourceClasses), this);
}
}
