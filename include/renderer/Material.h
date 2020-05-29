
#pragma once

#include "std.h"

#include "MaterialProgram.h"
#include "RenderableTexture.h"

#include "glm/glm.hpp"

class Material : public Resource
{
public:
    Material(ResourceRef<MaterialProgram> _program);
    Material(std::shared_ptr<Material> sourceMaterial);
    virtual ~Material();

    void use();
    void setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix);

    void setTexture(const std::string& textureName, const ResourceRef<RenderableTexture>& texture);

    void setBoolProperty(const std::string& name, bool value);
    void setIntProperty(const std::string& name, int value);
    void setFloatProperty(const std::string& name, float value);
    void setVec2Property(const std::string& name, const glm::vec2& value);
    void setVec3Property(const std::string& name, const glm::vec3& value);
    void setVec4Property(const std::string& name, const glm::vec4& value);
    void setProperty(const std::string& name, const void* data, uint size, GLenum matchType = 0);

    static std::shared_ptr<Resource> build(std::shared_ptr<void> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }
protected:
    Material() {}
    virtual std::vector<uint> getDependencies() override;
    virtual void resolveDependencies(ResolveMethod method) override;
    virtual bool load(std::shared_ptr<void> data) override;
private:
    struct PropInfo {
        PropInfo(bool value) : matchType(GL_BOOL), dataSize(1), data_bool(value) { }
        PropInfo(int value) : matchType(GL_INT), dataSize(sizeof(int)), data_int(value) { }
        PropInfo(float value) : matchType(GL_FLOAT), dataSize(sizeof(float)), data_float(value) { }
        PropInfo(const glm::vec2& value) : matchType(GL_FLOAT_VEC2), dataSize(sizeof(float) * 2), data_vec2(value) { }
        PropInfo(const glm::vec3& value) : matchType(GL_FLOAT_VEC3), dataSize(sizeof(float) * 3), data_vec3(value) { }
        PropInfo(const glm::vec4& value) : matchType(GL_FLOAT_VEC4), dataSize(sizeof(float) * 4), data_vec4(value) { }
    private:
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

        friend class Material;
    };

    ResourceRef<MaterialProgram> program;
    std::map<std::string, PropInfo> propValues;
    std::map<GLuint, ResourceRef<RenderableTexture>> textures;
    GLuint ubo = 0;

    struct BuildData
    {
        uint program;
        std::map<std::string, PropInfo> values;
        std::map<std::string, uint> textures;
    };
    
    static std::shared_ptr<Material> build(std::shared_ptr<BuildData> data);
};
