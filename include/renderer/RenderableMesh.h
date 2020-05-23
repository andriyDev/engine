
#pragma once

#include "std.h"

#include "resources/Mesh.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableMesh : public Resource
{
public:
    RenderableMesh();
    virtual ~RenderableMesh();

    void bind();
    void render();

protected:
    GLuint vao;
    GLuint buffers[2];
    uint bufferCount;
    uint indexCount;

    friend class RenderableMeshBuilder;
};

class RenderableMeshBuilder : public ResourceBuilder
{
public:
    RenderableMeshBuilder()
        : ResourceBuilder((uint)RenderResources::RenderableMesh) {}

    std::string sourceMesh;

    virtual std::shared_ptr<Resource> construct() override;

    virtual void init() override;

    virtual void startBuild() override;
};
