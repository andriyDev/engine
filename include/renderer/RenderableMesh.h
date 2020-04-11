
#pragma once

#include "std.h"

#include "resources/Mesh.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableMesh
{
public:
    RenderableMesh(Mesh* mesh);
    ~RenderableMesh();

    void bind();
    void render();

protected:
    GLuint vao;
    GLuint buffers[2];
    uint bufferCount;
    uint indexCount;
};
