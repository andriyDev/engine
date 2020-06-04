
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

class MaterialProgram : public Resource
{
public:
    MaterialProgram(vector<ResourceRef<Shader>> _vertexShaders,
        vector<ResourceRef<Shader>> _fragmentShaders);
    virtual ~MaterialProgram();

    void bind();

    void setMVP(mat4& modelMatrix, mat4& vpMatrix);

    GLint getUniformId(const string& uniformName) const;
    GLuint getProgramId() const;

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        vector<uint> vertexShaders;
        vector<uint> fragmentShaders;
    };

    static shared_ptr<BuildData> createAssetData(
        const vector<uint>& vertexShaders, const vector<uint>& fragmentShaders);
protected:
    MaterialProgram() {}

    virtual vector<uint> getDependencies() override;
    virtual void resolveDependencies(ResolveMethod method) override;
    virtual bool load(shared_ptr<Resource::BuildData> data) override;
private:
    static shared_ptr<MaterialProgram> build(shared_ptr<BuildData> data);

    GLuint ProgramId = 0;
    GLuint mvpLocation;
    GLuint modelMatrixLocation;

    vector<ResourceRef<Shader>> vertexShaders;
    vector<ResourceRef<Shader>> fragmentShaders;

    friend class MaterialProgramBuilder;
    friend class Material;
};
