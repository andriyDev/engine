
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

class MaterialProgram : public Resource
{
public:
    MaterialProgram(std::vector<ResourceRef<Shader>> _vertexShaders,
        std::vector<ResourceRef<Shader>> _fragmentShaders);
    virtual ~MaterialProgram();

    void bind();
    void useUBO(GLuint ubo);

    void setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix);

    GLuint createUBO();

    GLuint getUniformId(const std::string& uniformName) const;
    GLuint getProgramId() const;
    const std::map<std::string, std::pair<GLenum, GLuint>>& getUniformInfo() const;

    static std::shared_ptr<Resource> build(std::shared_ptr<Resource::BuildData> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        std::vector<uint> vertexShaders;
        std::vector<uint> fragmentShaders;
    };

    static std::shared_ptr<BuildData> createAssetData(
        const std::vector<uint>& vertexShaders, const std::vector<uint>& fragmentShaders);
protected:
    MaterialProgram() {}

    virtual std::vector<uint> getDependencies() override;
    virtual void resolveDependencies(ResolveMethod method) override;
    virtual bool load(std::shared_ptr<Resource::BuildData> data) override;
private:
    static std::shared_ptr<MaterialProgram> build(std::shared_ptr<BuildData> data);

    GLuint ProgramId = 0;

    std::map<std::string, std::pair<GLenum, GLuint>> uniforms;
    std::map<std::string, GLuint> textureIdMap;
    GLuint uboSize;
    GLuint uboLocation;
    GLuint mvpLocation;
    GLuint modelMatrixLocation;

    std::vector<ResourceRef<Shader>> vertexShaders;
    std::vector<ResourceRef<Shader>> fragmentShaders;

    friend class MaterialProgramBuilder;
    friend class Material;
};