
#pragma once

#include "std.h"

#include "utility/Serializer.h"
#include "resources/ResourceLoader.h"
#include "resources/FileResource.h"

#include <glm/glm.hpp>

class Mesh : public FileResource
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

    Vertex* vertData = nullptr;
    uint* indexData = nullptr;
    uint vertCount = 0;
    uint indexCount = 0;

    virtual ~Mesh();

    void clearData();

    static shared_ptr<Mesh> makeBox(vec3 extents=vec3(.5f,.5f,.5f));
protected:
    virtual void loadFromFile(ifstream& file) override;
    virtual void saveToFile(ofstream& file) override;
};
