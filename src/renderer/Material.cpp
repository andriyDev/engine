
#include "renderer/Material.h"

Material::Material(std::shared_ptr<MaterialProgram> _program)
    : program(_program)
{
    assert(program);
    programLoadEvent = program->triggerOnLoad(Material::build, this);
}

void Material::build(void* materialRaw, Resource::ResourceLoadDoneParams params)
{
    Material* material = static_cast<Material*>(materialRaw);
    material->programLoadEvent = 0;
    if(params.resource->state != Resource::Success) {
        return;
    }

    material->ubo = material->program->createUBO();
    material->usable = true;

    for(auto props : material->queuedProps) {
        material->setProperty(props.first, &props.second.data_float, props.second.dataSize, props.second.matchType);
    }
    material->queuedProps.clear();
}

Material::~Material()
{
    if(programLoadEvent) {
        if(program) {
            program->removeTriggerOnLoad(programLoadEvent);
        }
    }
    if(usable) {
        glDeleteBuffers(1, &ubo);
    }
}

void Material::use()
{
    if(usable) {
        program->bind();
        program->useUBO(ubo);
    }
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    if(usable) {
        program->setMVP(modelMatrix, vpMatrix);
    }
}

void Material::setBoolProperty(const std::string& name, bool value)
{
    if(usable) {
        setProperty(name, &value, 1, GL_BOOL);
    } else {
        PropInfo info = {GL_BOOL, 1};
        info.data_bool = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setIntProperty(const std::string& name, int value)
{
    if(usable) {
        setProperty(name, &value, sizeof(int), GL_INT);
    } else {
        PropInfo info = {GL_INT, sizeof(int)};
        info.data_int = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setFloatProperty(const std::string& name, float value)
{
    if(usable) {
        setProperty(name, &value, sizeof(float), GL_FLOAT);
    } else {
        PropInfo info = {GL_FLOAT, sizeof(float)};
        info.data_float = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setVec2Property(const std::string& name, const glm::vec2& value)
{
    if(usable) {
        setProperty(name, &value, sizeof(float)*2, GL_FLOAT_VEC2);
    } else {
        PropInfo info = {GL_FLOAT_VEC2, sizeof(float)*2};
        info.data_vec2 = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setVec3Property(const std::string& name, const glm::vec3& value)
{
    if(usable) {
        setProperty(name, &value, sizeof(float)*3, GL_FLOAT_VEC3);
    } else {
        PropInfo info = {GL_FLOAT_VEC3, sizeof(float)*3};
        info.data_vec3 = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setVec4Property(const std::string& name, const glm::vec4& value)
{
    if(usable) {
        setProperty(name, &value, sizeof(float)*4, GL_FLOAT_VEC4);
    } else {
        PropInfo info = {GL_FLOAT_VEC4, sizeof(float)*4};
        info.data_vec4 = value;
        queuedProps.insert_or_assign(name, info);
    }
}

void Material::setProperty(const std::string& name, const void* data, uint size, GLenum matchType)
{
    if(!usable) {
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
