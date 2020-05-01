
#pragma once

#include "std.h"

#include "utility/Serializer.h"

#include <glm/glm.hpp>

#include "RenderResources.h"

class Mesh
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

    ~Mesh();

    void clearData();
};

template<>
void write(Serializer& ser, const Mesh& mesh);

template<>
void read(Serializer& ser, Mesh& mesh);

void writeMesh(Serializer& ser, void* meshRaw);

void* readMesh(Serializer& ser);
