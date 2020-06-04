
#pragma once

#include "std.h"

#define GLEW_STATIC
#include <GL/glew.h>

class UIUtil
{
public:
    static void bindRectangle();
    static void renderRectangle();
private:
    ~UIUtil();

    static UIUtil data;
    GLuint rectangleMesh = 0;
    GLuint vertBuffer = 0;

    void buildMesh();
};
