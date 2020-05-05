
#pragma once

#include "std.h"

#include "MaterialProgram.h"

#include "glm/glm.hpp"

class Material : public Resource
{
public:
    Material() : Resource((uint)RenderResources::Material) {}
    ~Material();

    void use();
    void setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix);

    void setBoolProperty(const std::string& name, bool value);
    void setIntProperty(const std::string& name, int value);
    void setFloatProperty(const std::string& name, float value);
    void setVec2Property(const std::string& name, const glm::vec2& value);
    void setVec3Property(const std::string& name, const glm::vec3& value);
    void setVec4Property(const std::string& name, const glm::vec4& value);
    void setProperty(const std::string& name, const void* data, uint size, GLenum matchType = 0);
private:
    std::shared_ptr<MaterialProgram> program;
    std::map<std::string, std::pair<GLenum, GLuint>> uniforms;
    GLuint uboLocation;
    GLuint ubo;
    GLuint mvpLocation;

    friend class MaterialBuilder;
};

class MaterialBuilder : public ResourceBuilder
{
public:
    std::string materialProgram;

    MaterialBuilder() : ResourceBuilder((uint)RenderResources::Material) {}

    void setBoolProperty(const std::string& name, bool value);
    void setIntProperty(const std::string& name, int value);
    void setFloatProperty(const std::string& name, float value);
    void setVec2Property(const std::string& name, const glm::vec2& value);
    void setVec3Property(const std::string& name, const glm::vec3& value);
    void setVec4Property(const std::string& name, const glm::vec4& value);

    virtual std::shared_ptr<Resource> construct() override;

    virtual void init() override;

    virtual void startBuild() override;
private:
    std::map<std::string, bool> boolProps;
    std::map<std::string, int> intProps;
    std::map<std::string, float> floatProps;
    std::map<std::string, glm::vec2> vec2Props;
    std::map<std::string, glm::vec3> vec3Props;
    std::map<std::string, glm::vec4> vec4Props;
};
