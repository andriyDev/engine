
#pragma once

#include "std.h"

#include "MaterialProgram.h"

#include "glm/glm.hpp"
using namespace glm;

class UniformValue
{
public:
    UniformValue(string name, MaterialProgram* _program);

    virtual void setValue() = 0;
protected:
    GLuint location;
    MaterialProgram* program;
};

template<typename T>
class Uniform : public UniformValue
{
public:
    Uniform<T>(string name, MaterialProgram* _program) : UniformValue(name, _program) { }

    T* getValue() {
        return &value;
    }
    virtual void setValue() override;
protected:
    T value;
};

class Material
{
public:
    Material(MaterialProgram* _program);
    ~Material();

    void use();
    void setMVP(mat4& modelMatrix, mat4& vpMatrix);

    template<class U>
    U* addUniform(string name);
private:
    MaterialProgram* program;
    Uniform<mat4> mvp;
    map<string, UniformValue*> uniforms;
};
