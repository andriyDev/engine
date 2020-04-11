
#pragma once

#include "std.h"

#include "utility/Serializer.h"

#include <glm/glm.hpp>

using namespace glm;

#define RESOURCE_MESH 1

class Mesh
{
public:
    struct Vertex
    {
        vec3 position;
        vec4 colour;
        vec2 texCoord;
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
    };

    Vertex* vertData;
    uint* indexData;
    uint vertCount;
    uint indexCount;

    ~Mesh();

    void clearData();
};

template<>
void write(Serializer& ser, const Mesh& mesh);

template<>
void read(Serializer& ser, Mesh& mesh);

void writeMesh(Serializer& ser, void* meshRaw);

void* readMesh(Serializer& ser);
