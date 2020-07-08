
#pragma once

#include "std.h"

#define GLEW_STATIC
#include <GL/glew.h>

struct UIColour
{
    enum Type : uint
    {
        Base = 0,
        Hovered = 1,
        Active = 2
    };

    union {
        struct {
            vec4 base;
            vec4 hovered;
            vec4 active;
        };
        vec4 colours[3];
    };
    
    vec4 byType(Type type) { return colours[type]; }
};

class UIUtil
{
public:
    static void bindRectangle();
    static void renderRectangle();
    static void renderRectangles(uint rectangleCount);
private:
    ~UIUtil();

    static UIUtil data;
    GLuint rectangleMesh = 0;
    GLuint vertBuffer = 0;

    void buildMesh();
};
