
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"

class MaterialProgram
{
public:
    MaterialProgram(const vector<Shader*>& vertexShaderComponents,
        const vector<Shader*>& fragmentShaderComponents);
        
    ~MaterialProgram();

    void bind();
private:
    GLuint ProgramId;
};
