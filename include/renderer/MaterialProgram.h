
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"

class MaterialProgram
{
public:
    MaterialProgram(const std::vector<Shader*>& vertexShaderComponents,
        const std::vector<Shader*>& fragmentShaderComponents);
        
    ~MaterialProgram();

    void bind();

    GLuint getUniformId(const std::string& uniformName);
private:
    GLuint ProgramId;
};
