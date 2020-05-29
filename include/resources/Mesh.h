
#pragma once

#include "std.h"

#include "utility/Serializer.h"
#include "ResourceLoader.h"
#include "FileResource.h"

#include <glm/glm.hpp>

#include "RenderResources.h"

class Mesh : public FileResource
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

    Vertex* vertData = nullptr;
    uint* indexData = nullptr;
    uint vertCount = 0;
    uint indexCount = 0;

    virtual ~Mesh();

    void clearData();

    static std::shared_ptr<Mesh> makeBox(glm::vec3 extents=glm::vec3(.5f,.5f,.5f));
protected:
    virtual void loadFromFile(std::ifstream& file) override;
    virtual void saveToFile(std::ofstream& file) override;
};
