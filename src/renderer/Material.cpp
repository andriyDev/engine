
#include "renderer/Material.h"

UniformValue::UniformValue(std::string name, MaterialProgram* _program)
{
    program = _program;
    location = program->getUniformId(name);
}

template<>
void Uniform<float>::setValue()
{
    glUniform1f(location, value);
}

template<>
void Uniform<int>::setValue()
{
    glUniform1i(location, value);
}

template<>
void Uniform<glm::vec2>::setValue()
{
    glUniform2f(location, value[0], value[1]);
}

template<>
void Uniform<glm::vec3>::setValue()
{
    glUniform3f(location, value[0], value[1], value[2]);
}

template<>
void Uniform<glm::vec4>::setValue()
{
    glUniform4f(location, value[0], value[1], value[2], value[3]);
}

template<>
void Uniform<glm::mat4>::setValue()
{
    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

Material::Material(MaterialProgram* _program)
    : program(_program), mvp(Uniform<glm::mat4>("mvp", _program))
{ }

Material::~Material()
{
    for(auto p : uniforms) {
        delete p.second;
    }
}

void Material::use()
{
    program->bind();
    for(auto p : uniforms) {
        p.second->setValue();
    }
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    *mvp.getValue() = vpMatrix * modelMatrix;
    mvp.setValue();
}

template<class U>
U* Material::addUniform(std::string name)
{
    U* u = new U(name, program);
    uniforms.insert(std::make_pair<std::string, UniformValue*>(name, u));
    return u;
}
