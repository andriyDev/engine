
#include "renderer/MaterialProgram.h"

MaterialProgram::MaterialProgram(std::vector<ResourceRef<Shader>> _vertexShaders,
    std::vector<ResourceRef<Shader>> _fragmentShaders)
    : vertexShaders(_vertexShaders), fragmentShaders(_fragmentShaders)
{
    resolveDependencies(Immediate);
    if(!load(nullptr)) {
        throw "Failed to create MaterialProgram!";
    }
}

MaterialProgram::~MaterialProgram()
{
    if(ProgramId) {
        glDeleteProgram(ProgramId);
    }
}

void MaterialProgram::bind()
{
    glUseProgram(ProgramId);
}

void MaterialProgram::useUBO(GLuint ubo)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, uboLocation, ubo);
}

void MaterialProgram::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    glm::mat4 mvp = vpMatrix * modelMatrix;
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
}

GLuint MaterialProgram::createUBO()
{
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, (void*)0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, uboLocation, ubo);
    return ubo;
}

GLuint MaterialProgram::getUniformId(const std::string& uniformName) const
{
    return glGetUniformLocation(ProgramId, uniformName.c_str());
}

GLuint MaterialProgram::getProgramId() const
{
    return ProgramId;
}

const std::map<std::string, std::pair<GLenum, GLuint>>& MaterialProgram::getUniformInfo() const
{
    return uniforms;
}

void compileShader(GLuint shaderId, const std::vector<std::shared_ptr<Shader>>& components)
{
    GLint Result = GL_FALSE;
    int infoLength;

    std::vector<const char*> rawStrings;
    rawStrings.reserve(components.size());
    for(std::shared_ptr<Shader> shaderComp : components) {
        rawStrings.push_back(check(shaderComp)->code.c_str());
    }
    glShaderSource(shaderId, (GLsizei)rawStrings.size(), &rawStrings[0], NULL);
    glCompileShader(shaderId);

    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLength);
    if(infoLength > 0) {
        std::vector<char> errorMsg(infoLength + 1);
        glGetShaderInfoLog(shaderId, infoLength, NULL, &errorMsg[0]);
        fprintf(stderr, "Error (Shader Compilation):\n%s\n", &errorMsg[0]);
        return;
    }
}

#define MAX_UNIFORM_NAME_LEN 256

std::vector<uint> MaterialProgram::getDependencies()
{
    std::vector<uint> out;
    out.reserve(vertexShaders.size() + fragmentShaders.size());
    for(ResourceRef<Shader>& ref : vertexShaders) {
        out.push_back(ref);
    }
    for(ResourceRef<Shader>& ref : fragmentShaders) {
        out.push_back(ref);
    }
    return out;
}

void MaterialProgram::resolveDependencies(ResolveMethod method)
{
    for(ResourceRef<Shader>& ref : vertexShaders) {
        ref.resolve(method);
    }
    for(ResourceRef<Shader>& ref : fragmentShaders) {
        ref.resolve(method);
    }
}


bool MaterialProgram::load(std::shared_ptr<void> data)
{
    assert(!vertexShaders.empty());
    assert(!vertexShaders.empty());

    GLuint shaders[2];
    shaders[0] = glCreateShader(GL_VERTEX_SHADER);
    shaders[1] = glCreateShader(GL_FRAGMENT_SHADER);

    std::vector<std::shared_ptr<Shader>> vertexShaderComponents(vertexShaders.size());
    std::vector<std::shared_ptr<Shader>> fragmentShaderComponents(fragmentShaders.size());
    for(ResourceRef<Shader>& ref : vertexShaders) {
        vertexShaderComponents.push_back(ref.resolve(Immediate)); // Make sure these are all loaded.
    }
    for(ResourceRef<Shader>& ref : fragmentShaders) {
        fragmentShaderComponents.push_back(ref.resolve(Immediate)); // Make sure these are all loaded.
    }

    compileShader(shaders[0], vertexShaderComponents);
    compileShader(shaders[1], fragmentShaderComponents);

    ProgramId = glCreateProgram();
    glAttachShader(ProgramId, shaders[0]);
    glAttachShader(ProgramId, shaders[1]);
    glLinkProgram(ProgramId);

    GLint Result = GL_FALSE;
    int infoLength;

    glGetShaderiv(ProgramId, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(ProgramId, GL_INFO_LOG_LENGTH, &infoLength);
    if(infoLength > 0) {
        std::vector<char> errorMsg(infoLength + 1);
        glGetProgramInfoLog(ProgramId, infoLength, NULL, &errorMsg[0]);
        fprintf(stderr, "Error (Program Linking):\n%s\n", &errorMsg[0]);
        return;
    }

    glDetachShader(ProgramId, shaders[0]);
    glDetachShader(ProgramId, shaders[1]);
    
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    int uniformCount;
    glGetProgramiv(ProgramId, GL_ACTIVE_UNIFORMS, &uniformCount);

    int* propIndices = new int[uniformCount];
    int* propTypes = new int[uniformCount];
    for(int i = 0; i < uniformCount; i++) {
        propIndices[i] = i;
    }
    glGetActiveUniformsiv(ProgramId, uniformCount, (uint*)propIndices, GL_UNIFORM_TYPE, propTypes);

    int textures = 0;
    char buffer[MAX_UNIFORM_NAME_LEN];
    for(int i = 0; i < uniformCount; i++) {
        glGetActiveUniformName(ProgramId, i, MAX_UNIFORM_NAME_LEN, 0, buffer);

        std::string uniformName = buffer;
        if(propTypes[i] != GL_SAMPLER_2D) {
            continue;
        }
        GLuint loc = glGetUniformLocation(ProgramId, uniformName.c_str());
        glUniform1i(loc, textures);
        textureIdMap.insert(std::make_pair(uniformName, textures));
        textures++;
    }
    delete[] propIndices;
    delete[] propTypes;

    uint propsId = glGetUniformBlockIndex(ProgramId, "MaterialProps");
    if(propsId == GL_INVALID_INDEX) {
        glDeleteProgram(ProgramId);
        return false;
    }
    uboLocation = 0;

    glUniformBlockBinding(ProgramId, propsId, uboLocation);

    int propCount;
    glGetActiveUniformBlockiv(ProgramId, propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &propCount);

    propIndices = new int[propCount];
    glGetActiveUniformBlockiv(ProgramId, propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, propIndices);
    
    propTypes = new int[propCount];
    int* propOffsets = new int[propCount];
    
    glGetActiveUniformsiv(ProgramId, propCount, (uint*)propIndices, GL_UNIFORM_TYPE, propTypes);
    glGetActiveUniformsiv(ProgramId, propCount, (uint*)propIndices, GL_UNIFORM_OFFSET, propOffsets);

    for(int i = 0; i < propCount; i++) {
        glGetActiveUniformName(ProgramId, propIndices[i], MAX_UNIFORM_NAME_LEN, 0, buffer);

        std::string uniformName = buffer;
        GLenum uniformType = propTypes[i];
        GLuint uniformOffset = propOffsets[i];

        uniforms.insert(std::make_pair(uniformName, std::make_pair(uniformType, uniformOffset)));
    }

    delete[] propIndices;
    delete[] propTypes;
    delete[] propOffsets;
    
    int propsSize;
    glGetActiveUniformBlockiv(ProgramId, propsId, GL_UNIFORM_BLOCK_DATA_SIZE, &propsSize);
    uboSize = propsSize;

    mvpLocation = getUniformId("mvp");
    modelMatrixLocation = getUniformId("modelMatrix");

    // We no longer need these dependencies.
    vertexShaders.clear();
    fragmentShaders.clear();

    return true;
}

std::shared_ptr<MaterialProgram> MaterialProgram::build(std::shared_ptr<BuildData> data)
{
    std::shared_ptr<MaterialProgram> matProg = std::make_shared<MaterialProgram>();
    matProg->vertexShaders.reserve(data->vertexShaders.size());
    matProg->fragmentShaders.reserve(data->fragmentShaders.size());
    for(uint id : data->vertexShaders) {
        matProg->vertexShaders.push_back(id);
    }
    for(uint id : data->fragmentShaders) {
        matProg->fragmentShaders.push_back(id);
    }
    return matProg;
}

std::shared_ptr<MaterialProgram::BuildData> MaterialProgram::createAssetData(
    const std::vector<uint>& vertexShaders, const std::vector<uint>& fragmentShaders)
{
    std::shared_ptr<BuildData> data = std::make_shared<BuildData>();
    data->vertexShaders = vertexShaders;
    data->fragmentShaders = fragmentShaders;
    return data;
}
