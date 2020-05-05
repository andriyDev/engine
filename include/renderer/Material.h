
#pragma once

#include "std.h"

#include "MaterialProgram.h"

#include "glm/glm.hpp"

class Material
{
public:
    Material(std::shared_ptr<MaterialProgram> _program);
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
    bool usable = false;
    uint programLoadEvent = 0;
    GLuint ubo = 0;

    struct PropInfo {
        GLenum matchType;
        uint dataSize;
        union {
            bool data_bool;
            int data_int;
            float data_float;
            glm::vec2 data_vec2;
            glm::vec3 data_vec3;
            glm::vec4 data_vec4;
        };
    };

    std::map<std::string, PropInfo> queuedProps;

    static void build(void* materialRaw, Resource::ResourceLoadDoneParams params);
};
