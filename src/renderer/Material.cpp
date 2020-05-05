
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
        glBindBufferBase(GL_UNIFORM_BUFFER, uboLocation, ubo);
    }
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    glm::mat4 mvp = vpMatrix * modelMatrix;
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
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

    uint propsId = glGetUniformBlockIndex(program->getProgramId(), "MaterialProps");
    if(propsId == GL_INVALID_INDEX) {
        target->state = Resource::Failure;
        return;
    }
    target->uboLocation = 0;

    glUniformBlockBinding(program->getProgramId(), propsId, 0);

    int propCount;
    glGetActiveUniformBlockiv(program->getProgramId(), propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &propCount);

    int* propIndices = new int[propCount];
    glGetActiveUniformBlockiv(program->getProgramId(), propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, propIndices);
    
    int* propTypes = new int[propCount];
    int* propOffsets = new int[propCount];
    
    glGetActiveUniformsiv(program->getProgramId(), propCount, (uint*)propIndices, GL_UNIFORM_TYPE, propTypes);
    glGetActiveUniformsiv(program->getProgramId(), propCount, (uint*)propIndices, GL_UNIFORM_OFFSET, propOffsets);

    char buffer[256];
    for(int i = 0; i < propCount; i++) {
        glGetActiveUniformName(program->getProgramId(), propIndices[i], 256, 0, buffer);

        std::string uniformName = buffer;
        GLenum uniformType = propTypes[i];
        GLuint uniformOffset = propOffsets[i];

        target->uniforms.insert(std::make_pair(uniformName, std::make_pair(uniformType, uniformOffset)));
    }
    delete[] propIndices;
    delete[] propTypes;
    delete[] propOffsets;
    
    int propsSize;
    glGetActiveUniformBlockiv(program->getProgramId(), propsId, GL_UNIFORM_BLOCK_DATA_SIZE, &propsSize);

    target->mvpLocation = program->getUniformId("mvp");

    glGenBuffers(1, &target->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, target->ubo);
    glBufferData(GL_UNIFORM_BUFFER, propsSize, (void*)0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, target->ubo);

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

    target->state = Resource::Success;
}
