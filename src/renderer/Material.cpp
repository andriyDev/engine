
#include "renderer/Material.h"

Material::Material(MaterialProgram* _program)
    : program(_program), specifier(nullptr)
{}

void Material::use(float tickPercent, const mat4& VPMatrix)
{
    program->bind();
    if(specifier) {
        specifier->setUniforms(program, tickPercent, VPMatrix);
    }
}
