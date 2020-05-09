
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

class MaterialProgram : public Resource
{
public:
    MaterialProgram() : Resource((uint)RenderResources::MaterialProgram) {}
        
    ~MaterialProgram();

    void bind();
    void useUBO(GLuint ubo);

    void setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix);

    GLuint createUBO();

    GLuint getUniformId(const std::string& uniformName) const;
    GLuint getProgramId() const;
    const std::map<std::string, std::pair<GLenum, GLuint>>& getUniformInfo() const;
private:
    GLuint ProgramId;

    std::map<std::string, std::pair<GLenum, GLuint>> uniforms;
    std::map<std::string, GLuint> textureIdMap;
    GLuint uboSize;
    GLuint uboLocation;
    GLuint mvpLocation;
    GLuint modelMatrixLocation;

    friend class MaterialProgramBuilder;
    friend class Material;
};

class MaterialProgramBuilder : public ResourceBuilder
{
public:
    MaterialProgramBuilder() : ResourceBuilder((uint)RenderResources::MaterialProgram) {}

    std::vector<std::string> vertexComponents;
    std::vector<std::string> fragmentComponents;

    virtual std::shared_ptr<Resource> construct() override;

    virtual void init() override;
    
    virtual void startBuild() override;
};
