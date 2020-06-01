
#include "renderer/Material.h"

Material::Material(ResourceRef<MaterialProgram> _program)
    : program(_program)
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    assert(prog);
    ubo = prog->createUBO();
}

Material::Material(std::shared_ptr<Material> sourceMaterial)
{
    assert(sourceMaterial);
    program = sourceMaterial->program;
    propValues = sourceMaterial->propValues;
    textures = sourceMaterial->textures;
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    ubo = prog->createUBO();
}

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
    for(auto& tex_pair : textures) {
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
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

void Material::setIntProperty(const std::string& name, int value)
{
    PropInfo info(value);
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

void Material::setFloatProperty(const std::string& name, float value)
{
    PropInfo info(value);
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

void Material::setVec2Property(const std::string& name, const glm::vec2& value)
{
    PropInfo info(value);
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

void Material::setVec3Property(const std::string& name, const glm::vec3& value)
{
    PropInfo info(value);
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

void Material::setVec4Property(const std::string& name, const glm::vec4& value)
{
    PropInfo info(value);
    // Only store the value if we were able to set it in the shader.
    if(setProperty(name, &info.data_float, info.dataSize, info.matchType)) {
        propValues.insert_or_assign(name, info);
    }
}

bool Material::setProperty(const std::string& name, const void* data, uint size, GLenum matchType)
{
    std::shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    const auto& uniforms = prog->getUniformInfo();
    auto it = uniforms.find(name);
    if(it == uniforms.end() || matchType != 0 && it->second.first != matchType) {
        return true; // Cannot set uniform.
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, it->second.second, size, data);
    return false;
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
    for(auto& p : textures) {
        p.second.resolve(method);
    }
}

bool Material::load(std::shared_ptr<Resource::BuildData> data)
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
    return true;
}

std::shared_ptr<Material> Material::build(std::shared_ptr<BuildData> data)
{
    std::shared_ptr<Material> mat(new Material());
    mat->program = data->program;
    // We will use the material's texture map as a cache to hold the references waiting to be filled.
    for(auto p : data->textures) {
        mat->textures.insert(std::make_pair(p.second, p.second));
    }
    return mat;
}

std::shared_ptr<Material::BuildData> Material::createAssetData(uint programId)
{
    std::shared_ptr<BuildData> data = std::make_shared<BuildData>();
    data->program = programId;
    return data;
}

void Material::BuildData::setTexture(const std::string& textureName, uint textureId)
{
    textures.insert_or_assign(textureName, textureId);
}

void Material::BuildData::setBoolProperty(const std::string& name, bool value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setIntProperty(const std::string& name, int value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setFloatProperty(const std::string& name, float value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec2Property(const std::string& name, const glm::vec2& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec3Property(const std::string& name, const glm::vec3& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec4Property(const std::string& name, const glm::vec4& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}
