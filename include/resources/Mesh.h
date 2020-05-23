
#pragma once

#include "std.h"

#include "utility/Serializer.h"
#include "ResourceLoader.h"
#include "FileResourceBuilder.h"

#include <glm/glm.hpp>

#include "RenderResources.h"

class Mesh : public Resource
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 colour;
        glm::vec2 texCoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    Vertex* vertData;
    uint* indexData;
    uint vertCount;
    uint indexCount;

    Mesh();
    virtual ~Mesh();

    void clearData();

    static std::shared_ptr<Mesh> makeBox(glm::vec3 extents=glm::vec3(.5f,.5f,.5f));
};

class MeshBuilder : public FileResourceBuilder<Mesh>
{
public:
    MeshBuilder(std::string resourceName, std::shared_ptr<PackageFile> resourcePackage)
    : FileResourceBuilder((uint)RenderResources::Mesh, resourcePackage, resourceName, (uint)FileRenderResources::Mesh)
    {}
};

template<>
void write(Serializer& ser, const Mesh& mesh);

template<>
void read(Serializer& ser, Mesh& mesh);

void writeMesh(Serializer& ser, void* meshRaw);

void* readMesh(Serializer& ser);

void readIntoMesh(Serializer& ser, void* meshRaw);
