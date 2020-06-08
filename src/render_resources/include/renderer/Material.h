
#pragma once

#include "std.h"

#include "MaterialProgram.h"
#include "RenderableTexture.h"

#include "glm/glm.hpp"

class Material : public Resource
{
public:
    Material(ResourceRef<MaterialProgram> _program);
    Material(shared_ptr<Material> sourceMaterial);
    virtual ~Material();

    void use();
    void setMVP(mat4& modelMatrix, mat4& vpMatrix);
    
    GLint getUniformId(const string& uniformName);

    void setTexture(const string& textureName, const ResourceRef<RenderableTexture>& texture);

    void setIntProperty(const string& name, int value, bool temporary = false);
    void setFloatProperty(const string& name, float value, bool temporary = false);
    void setVec2Property(const string& name, const vec2& value, bool temporary = false);
    void setVec3Property(const string& name, const vec3& value, bool temporary = false);
    void setVec4Property(const string& name, const vec4& value, bool temporary = false);

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    struct PropInfo {
        PropInfo(int value) : matchType(GL_INT), dataSize(sizeof(int)), data_int(value) { }
        PropInfo(float value) : matchType(GL_FLOAT), dataSize(sizeof(float)), data_float(value) { }
        PropInfo(const vec2& value) : matchType(GL_FLOAT_VEC2), dataSize(sizeof(float) * 2), data_vec2(value) { }
        PropInfo(const vec3& value) : matchType(GL_FLOAT_VEC3), dataSize(sizeof(float) * 3), data_vec3(value) { }
        PropInfo(const vec4& value) : matchType(GL_FLOAT_VEC4), dataSize(sizeof(float) * 4), data_vec4(value) { }
    private:
        GLenum matchType;
        uint dataSize;
        union {
            int data_int;
            float data_float;
            vec2 data_vec2;
            vec3 data_vec3;
            vec4 data_vec4;
        };
        
        void use(uint uniformLocation);

        friend class Material;
    };

    class BuildData : public Resource::BuildData
    {
    public:
        uint program;

        void setTexture(const string& textureName, uint textureId);

        void setIntProperty(const string& name, int value);
        void setFloatProperty(const string& name, float value);
        void setVec2Property(const string& name, const vec2& value);
        void setVec3Property(const string& name, const vec3& value);
        void setVec4Property(const string& name, const vec4& value);
    private:
        hash_map<string, PropInfo> values;
        hash_map<string, uint> textures;

        friend class Material;
    };

    static shared_ptr<BuildData> createAssetData(uint programId);
protected:
    Material() {}

    virtual vector<uint> getDependencies() override;
    virtual void resolveDependencies(ResolveMethod method) override;
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

    void setProperty(const string& name, PropInfo& value, bool temporary = false);
private:
    ResourceRef<MaterialProgram> program;
    hash_map<string, GLint> propMap;
    hash_map<GLuint, PropInfo> values;
    vector<ResourceRef<RenderableTexture>> textures;
    
    static shared_ptr<Material> build(shared_ptr<BuildData> data);
};
