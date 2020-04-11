
#pragma once

#include "std.h"

#include "MaterialProgram.h"

#include "glm/glm.hpp"
using namespace glm;

class UniformSpecifier
{
public:
    virtual void setUniforms(MaterialProgram* Program, float tickPercent, const mat4& VPMatrix) = 0;
};

class Material
{
public:
    UniformSpecifier* specifier;
    
    Material(MaterialProgram* _program);

    void use(float tickPercent, const mat4& VPMatrix);
private:
    MaterialProgram* program;
};
