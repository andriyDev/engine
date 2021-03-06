
#include "renderer/Material.h"

void Material::PropInfo::use(uint uniformLocation)
{
    switch (matchType)
    {
    case GL_FLOAT:
        glUniform1f(uniformLocation, data_float);
        break;
    case GL_INT:
        glUniform1i(uniformLocation, data_int);
        break;
    case GL_FLOAT_VEC2:
        glUniform2fv(uniformLocation, 1, &data_vec2[0]);
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(uniformLocation, 1, &data_vec3[0]);
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(uniformLocation, 1, &data_vec4[0]);
        break;
    }
}

Material::Material(ResourceRef<MaterialProgram> _program)
    : program(_program)
{
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    assert(prog);
}

Material::Material(shared_ptr<Material> sourceMaterial)
{
    assert(sourceMaterial);
    program = sourceMaterial->program;
    propMap = sourceMaterial->propMap;
    values = sourceMaterial->values;
    textures = sourceMaterial->textures;
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
}

Material::~Material()
{

}

void Material::use()
{
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    prog->bind();
    for(auto& val_pair : values) {
        val_pair.second.use(val_pair.first);
    }
    for(int i = 0; i < textures.size(); i++) {
        shared_ptr<RenderableTexture> tex = textures[i].resolve(Immediate);
        if(tex) {
            tex->bind(i);
        }
    }
}

GLint Material::getUniformId(const string& uniformName)
{
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    if(!prog) {
        return -1;
    }
    auto it = propMap.find(uniformName);
    if(it == propMap.end()) {
        GLint id = prog->getUniformId(uniformName);
        if(id == -1) {
            return -1;
        }
        it = propMap.insert(make_pair(uniformName, id)).first;
    }
    return it->second;
}

void Material::setTexture(const string& textureName, ResourceRef<RenderableTexture>& texture, bool temporary)
{
    GLint uniformId = getUniformId(textureName);
    if(uniformId == -1) {
        return;
    }

    auto it = values.find(uniformId);
    if(it == values.end()) {
        int textureUnit = (int)textures.size();
        values.insert_or_assign(uniformId, PropInfo(textureUnit));
        if(temporary) {
            textures.push_back(ResourceRef<RenderableTexture>(nullptr));
            shared_ptr<RenderableTexture> texPtr = texture.resolve(Deferred);
            if(texPtr) {
                texPtr->bind(textureUnit);
            }
        } else {
            textures.push_back(texture);
        }
    } else {
        if(it->second.data_int < textures.size()) {
            if(temporary) {
                textures[it->second.data_int] = ResourceRef<RenderableTexture>(nullptr);
                shared_ptr<RenderableTexture> texPtr = texture.resolve(Deferred);
                if(texPtr) {
                    texPtr->bind(it->second.data_int);
                }
            } else {
                textures[it->second.data_int] = texture;
            }
        }
    }
}

void Material::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);
    prog->setMVP(modelMatrix, vpMatrix);
}

void Material::setIntProperty(const string& name, int value, bool temporary)
{
    setProperty(name, PropInfo(value), temporary);
}

void Material::setFloatProperty(const string& name, float value, bool temporary)
{
    setProperty(name, PropInfo(value), temporary);
}

void Material::setVec2Property(const string& name, const glm::vec2& value, bool temporary)
{
    setProperty(name, PropInfo(value), temporary);
}

void Material::setVec3Property(const string& name, const glm::vec3& value, bool temporary)
{
    setProperty(name, PropInfo(value), temporary);
}

void Material::setVec4Property(const string& name, const glm::vec4& value, bool temporary)
{
    setProperty(name, PropInfo(value), temporary);
}

void Material::setProperty(const string& name, PropInfo& value, bool temporary)
{
    PropInfo info(value);
    GLint uniformId = getUniformId(name);
    if(uniformId == -1) {
        return;
    }
    
    if(temporary) {
        value.use(uniformId);
    } else {
        values.insert_or_assign(uniformId, value);
    }
}

vector<uint> Material::getDependencies()
{
    vector<uint> out;
    out.push_back(program);
    for(auto& tex : textures) {
        out.push_back(tex);
    }
    return out;
}

void Material::resolveDependencies(ResolveMethod method)
{
    program.resolve(method);
    for(auto& tex : textures) {
        tex.resolve(method);
    }
}

bool Material::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
    shared_ptr<MaterialProgram> prog = program.resolve(Immediate);

    vector<ResourceRef<RenderableTexture>> resolvedTextures = textures;
    hash_map<GLuint, PropInfo> texMap = values;
    textures.clear();
    values.clear();
    for(auto props : buildData->values) {
        setProperty(props.first, props.second, false);
    }
    for(auto tex : buildData->textures) {
        setTexture(tex.first, resolvedTextures[texMap.find(tex.second)->second.data_int]);
    }
    return true;
}

shared_ptr<Material> Material::build(shared_ptr<BuildData> data)
{
    shared_ptr<Material> mat(new Material());
    mat->program = data->program;
    // We will use the material's texture array to store texture dependencies,
    // and the value map to map texture ids to their index.
    int index = 0;
    for(auto texId : data->textures) {
        mat->values.insert(make_pair(texId.second, PropInfo(index)));
        mat->textures.push_back(index);
        index++;
    }
    return mat;
}

shared_ptr<Material::BuildData> Material::createAssetData(uint programId)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->program = programId;
    return data;
}

void Material::BuildData::setTexture(const string& textureName, uint textureId)
{
    textures.insert_or_assign(textureName, textureId);
}

void Material::BuildData::setIntProperty(const string& name, int value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setFloatProperty(const string& name, float value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec2Property(const string& name, const glm::vec2& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec3Property(const string& name, const glm::vec3& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}

void Material::BuildData::setVec4Property(const string& name, const glm::vec4& value)
{
    values.insert_or_assign(name, Material::PropInfo(value));
}
