
#pragma once

#include "std.h"

#include "MaterialProgram.h"

#include "glm/glm.hpp"

class UniformValue
{
public:
    UniformValue(std::string name, MaterialProgram* _program);

    virtual void setValue() = 0;
protected:
    GLuint location;
    MaterialProgram* program;
};

template<typename T>
class Uniform : public UniformValue
{
public:
    Uniform<T>(std::string name, MaterialProgram* _program) : UniformValue(name, _program) { }

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
    void setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix);

    template<class U>
    U* addUniform(std::string name);
private:
    MaterialProgram* program;
    Uniform<glm::mat4> mvp;
    std::map<std::string, UniformValue*> uniforms;
};
