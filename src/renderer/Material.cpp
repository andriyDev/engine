
#include "renderer/Material.h"

Material::~Material()
{
    if(ubo) {
        glDeleteBuffers(1, &ubo);
    }
}

void Material::use()
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    prog->bind();
    prog->useUBO(ubo);
    for(auto tex_pair : textures) {
        std::shared_ptr<RenderableTexture> tex = tex_pair.second.resolve(Immediate);
        tex->bind(tex_pair.first);
    }
}

void Material::setTexture(const std::string& textureName, const ResourceRef<RenderableTexture>& texture)
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    auto it = prog->textureIdMap.find(textureName);
    if(it == prog->textureIdMap.end()) {
        return; // Cannot set texture.
    }
    GLuint id = it->second;
    textures.insert_or_assign(id, texture);
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    prog->setMVP(modelMatrix, vpMatrix);
}

void Material::setBoolProperty(const std::string& name, bool value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setIntProperty(const std::string& name, int value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setFloatProperty(const std::string& name, float value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setVec2Property(const std::string& name, const glm::vec2& value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setVec3Property(const std::string& name, const glm::vec3& value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setVec4Property(const std::string& name, const glm::vec4& value)
{
    PropInfo info(value);
    propValues.insert_or_assign(name, info);
    setProperty(name, &info.data_float, info.dataSize, info.matchType);
}

void Material::setProperty(const std::string& name, const void* data, uint size, GLenum matchType)
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    const auto& uniforms = prog->getUniformInfo();
    auto it = uniforms.find(name);
    if(it == uniforms.end() || matchType != 0 && it->second.first != matchType) {
        throw "Bad uniform! Either uniform not found or it does not match the desired type!";
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, it->second.second, size, data);
}

std::vector<uint> Material::getDependencies()
{
    std::vector<uint> out;
    out.push_back(program);
    for(auto p : textures) {
        out.push_back(p.second);
    }
    return out;
}

void Material::resolveDependencies(ResolveMethod method)
{
    program.resolve(method);
    for(auto p : textures) {
        p.second.resolve(method);
    }
}

bool Material::load(std::shared_ptr<void> data)
{
    std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    ubo = prog->createUBO();

    std::map<uint, ResourceRef<RenderableTexture>> resolvedTextures = textures;
    textures.clear();
    for(auto props : buildData->values) {
        setProperty(props.first, &props.second.data_float, props.second.dataSize, props.second.matchType);
    }
    for(auto tex : buildData->textures) {
        setTexture(tex.first, resolvedTextures.find(tex.second)->second);
    }
}

std::shared_ptr<Material> Material::build(std::shared_ptr<BuildData> data)
{
    std::shared_ptr<Material> mat = std::make_shared<Material>();
    mat->program = data->program;
    // We will use the material's texture map as a cache to hold the references waiting to be filled.
    for(auto p : data->textures) {
        mat->textures.insert(std::make_pair(p.second, p.second));
    }
    return mat;
}
