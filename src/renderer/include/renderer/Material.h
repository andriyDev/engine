
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

    void setIntProperty(const std::string& name, int value, bool temporary = false);
    void setFloatProperty(const std::string& name, float value, bool temporary = false);
    void setVec2Property(const std::string& name, const glm::vec2& value, bool temporary = false);
    void setVec3Property(const std::string& name, const glm::vec3& value, bool temporary = false);
    void setVec4Property(const std::string& name, const glm::vec4& value, bool temporary = false);

    static std::shared_ptr<Resource> build(std::shared_ptr<Resource::BuildData> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    struct PropInfo {
        PropInfo(int value) : matchType(GL_INT), dataSize(sizeof(int)), data_int(value) { }
        PropInfo(float value) : matchType(GL_FLOAT), dataSize(sizeof(float)), data_float(value) { }
        PropInfo(const glm::vec2& value) : matchType(GL_FLOAT_VEC2), dataSize(sizeof(float) * 2), data_vec2(value) { }
        PropInfo(const glm::vec3& value) : matchType(GL_FLOAT_VEC3), dataSize(sizeof(float) * 3), data_vec3(value) { }
        PropInfo(const glm::vec4& value) : matchType(GL_FLOAT_VEC4), dataSize(sizeof(float) * 4), data_vec4(value) { }
    private:
        GLenum matchType;
        uint dataSize;
        union {
            int data_int;
            float data_float;
            glm::vec2 data_vec2;
            glm::vec3 data_vec3;
            glm::vec4 data_vec4;
        };
        
        void use(uint uniformLocation);

        friend class Material;
    };

    class BuildData : public Resource::BuildData
    {
    public:
        uint program;

        void setTexture(const std::string& textureName, uint textureId);

        void setIntProperty(const std::string& name, int value);
        void setFloatProperty(const std::string& name, float value);
        void setVec2Property(const std::string& name, const glm::vec2& value);
        void setVec3Property(const std::string& name, const glm::vec3& value);
        void setVec4Property(const std::string& name, const glm::vec4& value);
    private:
        std::unordered_map<std::string, PropInfo> values;
        std::unordered_map<std::string, uint> textures;

        friend class Material;
    };

    static std::shared_ptr<BuildData> createAssetData(uint programId);
protected:
    Material() {}

    virtual std::vector<uint> getDependencies() override;
    virtual void resolveDependencies(ResolveMethod method) override;
    virtual bool load(std::shared_ptr<Resource::BuildData> data) override;

    void setProperty(const std::string& name, PropInfo& value, bool temporary = false);
private:
    ResourceRef<MaterialProgram> program;
    std::unordered_map<std::string, GLuint> propMap;
    std::unordered_map<GLuint, PropInfo> values;
    std::vector<ResourceRef<RenderableTexture>> textures;
    
    static std::shared_ptr<Material> build(std::shared_ptr<BuildData> data);
};
