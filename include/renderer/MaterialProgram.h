
#pragma once

#include "std.h"
#include "resources/Shader.h"

#define GLEW_STATIC
#include "GL/glew.h"

class MaterialProgram : public Resource
{
public:
    MaterialProgram() : Resource((uint)RenderResources::MaterialProgram) {}
        
    ~MaterialProgram();

    void bind();

    GLuint getUniformId(const std::string& uniformName);
private:
    GLuint ProgramId;

    friend class MaterialProgramBuilder;
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
