
#include "renderer/Material.h"

Material::~Material()
{
    if(state == Success) {
        glDeleteBuffers(1, &ubo);
    }
}

void Material::use()
{
    if(state == Success) {
        program->bind();
        program->useUBO(ubo);
    }
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    if(state == Success) {
        program->setMVP(modelMatrix, vpMatrix);
    }
}

void Material::setBoolProperty(const std::string& name, bool value)
{
    setProperty(name, &value, 1, GL_BOOL);
}

void Material::setIntProperty(const std::string& name, int value)
{
    setProperty(name, &value, sizeof(int), GL_INT);
}

void Material::setFloatProperty(const std::string& name, float value)
{
    setProperty(name, &value, sizeof(float), GL_FLOAT);
}

void Material::setVec2Property(const std::string& name, const glm::vec2& value)
{
    setProperty(name, &value, sizeof(float)*2, GL_FLOAT_VEC2);
}

void Material::setVec3Property(const std::string& name, const glm::vec3& value)
{
    setProperty(name, &value, sizeof(float)*3, GL_FLOAT_VEC3);
}

void Material::setVec4Property(const std::string& name, const glm::vec4& value)
{
    setProperty(name, &value, sizeof(float)*4, GL_FLOAT_VEC4);
}

void Material::setProperty(const std::string& name, const void* data, uint size, GLenum matchType)
{
    if(state != Success) {
        return;
    }
    const auto& uniforms = program->getUniformInfo();
    auto it = uniforms.find(name);
    if(it == uniforms.end() || matchType != 0 && it->second.first != matchType) {
        throw "Bad uniform! Either uniform not found or it does not match the desired type!";
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, it->second.second, size, data);
}

std::shared_ptr<Resource> MaterialBuilder::construct()
{
    return std::make_shared<Material>();
}

void MaterialBuilder::init()
{
    addDependency(materialProgram);
}

void MaterialBuilder::setBoolProperty(const std::string& name, bool value)
{
    boolProps.insert_or_assign(name, value);
}

void MaterialBuilder::setIntProperty(const std::string& name, int value)
{
    intProps.insert_or_assign(name, value);
}

void MaterialBuilder::setFloatProperty(const std::string& name, float value)
{
    floatProps.insert_or_assign(name, value);
}

void MaterialBuilder::setVec2Property(const std::string& name, const glm::vec2& value)
{
    vec2Props.insert_or_assign(name, value);
}

void MaterialBuilder::setVec3Property(const std::string& name, const glm::vec3& value)
{
    vec3Props.insert_or_assign(name, value);
}

void MaterialBuilder::setVec4Property(const std::string& name, const glm::vec4& value)
{
    vec4Props.insert_or_assign(name, value);
}

void MaterialBuilder::startBuild()
{
    std::shared_ptr<Material> target = getResource<Material>();
    std::shared_ptr<MaterialProgram> program = getDependency<MaterialProgram>(materialProgram,
        (uint)RenderResources::MaterialProgram);
    assert(program);
    target->program = program;

    target->ubo = program->createUBO();

    target->state = Resource::Success;

    for(auto p : boolProps) {
        target->setBoolProperty(p.first, p.second);
    }
    for(auto p : intProps) {
        target->setIntProperty(p.first, p.second);
    }
    for(auto p : floatProps) {
        target->setFloatProperty(p.first, p.second);
    }
    for(auto p : vec2Props) {
        target->setVec2Property(p.first, p.second);
    }
    for(auto p : vec3Props) {
        target->setVec3Property(p.first, p.second);
    }
    for(auto p : vec4Props) {
        target->setVec4Property(p.first, p.second);
    }
}
