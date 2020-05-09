
#include "renderer/MaterialProgram.h"

MaterialProgram::~MaterialProgram()
{
    if(state == Success) {
        glDeleteProgram(ProgramId);
    }
}

void MaterialProgram::bind()
{
    if(state == Success) {
        glUseProgram(ProgramId);
    }
}

void MaterialProgram::useUBO(GLuint ubo)
{
    if(state == Success) {
        glBindBufferBase(GL_UNIFORM_BUFFER, uboLocation, ubo);
    }
}

void MaterialProgram::setMVP(glm::mat4& modelMatrix, glm::mat4& vpMatrix)
{
    if(state == Success) {
        glm::mat4 mvp = vpMatrix * modelMatrix;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    }
}

GLuint MaterialProgram::createUBO()
{
    if(state != Success) {
        return 0;
    }
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, (void*)0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, uboLocation, ubo);
    return ubo;
}

GLuint MaterialProgram::getUniformId(const std::string& uniformName) const
{
    return state == Success ? glGetUniformLocation(ProgramId, uniformName.c_str()) : 0;
}

GLuint MaterialProgram::getProgramId() const
{
    return ProgramId;
}

const std::map<std::string, std::pair<GLenum, GLuint>>& MaterialProgram::getUniformInfo() const
{
    return uniforms;
}

std::shared_ptr<Resource> MaterialProgramBuilder::construct()
{
    return std::make_shared<MaterialProgram>();
}

void MaterialProgramBuilder::init()
{
    for(std::string component : vertexComponents) {
        addDependency(component);
    }
    for(std::string component : fragmentComponents) {
        addDependency(component);
    }
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

void MaterialProgramBuilder::startBuild()
{
    std::shared_ptr<MaterialProgram> target = getResource<MaterialProgram>();

    assert(!vertexComponents.empty());
    assert(!fragmentComponents.empty());

    std::vector<std::shared_ptr<Shader>> vertexShaderComponents;
    std::vector<std::shared_ptr<Shader>> fragmentShaderComponents;
    for(std::string shaderName : vertexComponents) {
        if(auto ptr = getDependency<Shader>(shaderName, (uint)RenderResources::Shader)) {
            vertexShaderComponents.push_back(ptr);
        }
    }
    for(std::string shaderName : fragmentComponents) {
        if(auto ptr = getDependency<Shader>(shaderName, (uint)RenderResources::Shader)) {
            fragmentShaderComponents.push_back(ptr);
        }
    }

    assert(!vertexShaderComponents.empty());
    assert(!fragmentShaderComponents.empty());

    GLuint shaders[2];
    shaders[0] = glCreateShader(GL_VERTEX_SHADER);
    shaders[1] = glCreateShader(GL_FRAGMENT_SHADER);

    compileShader(shaders[0], vertexShaderComponents);
    compileShader(shaders[1], fragmentShaderComponents);

    target->ProgramId = glCreateProgram();
    glAttachShader(target->ProgramId, shaders[0]);
    glAttachShader(target->ProgramId, shaders[1]);
    glLinkProgram(target->ProgramId);

    GLint Result = GL_FALSE;
    int infoLength;

    glGetShaderiv(target->ProgramId, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(target->ProgramId, GL_INFO_LOG_LENGTH, &infoLength);
    if(infoLength > 0) {
        std::vector<char> errorMsg(infoLength + 1);
        glGetProgramInfoLog(target->ProgramId, infoLength, NULL, &errorMsg[0]);
        fprintf(stderr, "Error (Program Linking):\n%s\n", &errorMsg[0]);
        return;
    }

    glDetachShader(target->ProgramId, shaders[0]);
    glDetachShader(target->ProgramId, shaders[1]);
    
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    uint propsId = glGetUniformBlockIndex(target->ProgramId, "MaterialProps");
    if(propsId == GL_INVALID_INDEX) {
        glDeleteProgram(target->ProgramId);
        target->state = Resource::Failure;
        return;
    }
    target->uboLocation = 0;

    glUniformBlockBinding(target->ProgramId, propsId, target->uboLocation);

    int propCount;
    glGetActiveUniformBlockiv(target->ProgramId, propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &propCount);

    int* propIndices = new int[propCount];
    glGetActiveUniformBlockiv(target->ProgramId, propsId, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, propIndices);
    
    int* propTypes = new int[propCount];
    int* propOffsets = new int[propCount];
    
    glGetActiveUniformsiv(target->ProgramId, propCount, (uint*)propIndices, GL_UNIFORM_TYPE, propTypes);
    glGetActiveUniformsiv(target->ProgramId, propCount, (uint*)propIndices, GL_UNIFORM_OFFSET, propOffsets);

    char buffer[MAX_UNIFORM_NAME_LEN];
    for(int i = 0; i < propCount; i++) {
        glGetActiveUniformName(target->ProgramId, propIndices[i], MAX_UNIFORM_NAME_LEN, 0, buffer);

        std::string uniformName = buffer;
        GLenum uniformType = propTypes[i];
        GLuint uniformOffset = propOffsets[i];

        target->uniforms.insert(std::make_pair(uniformName, std::make_pair(uniformType, uniformOffset)));
    }

    delete[] propIndices;
    delete[] propTypes;
    delete[] propOffsets;
    
    int propsSize;
    glGetActiveUniformBlockiv(target->ProgramId, propsId, GL_UNIFORM_BLOCK_DATA_SIZE, &propsSize);
    target->uboSize = propsSize;

    int uniformCount;
    glGetProgramiv(target->ProgramId, GL_ACTIVE_UNIFORMS, &uniformCount);

    propIndices = new int[uniformCount];
    propTypes = new int[uniformCount];
    for(int i = 0; i < uniformCount; i++) {
        propIndices[i] = i;
    }
    glGetActiveUniformsiv(target->ProgramId, uniformCount, (uint*)propIndices, GL_UNIFORM_TYPE, propTypes);

    int textures = 0;
    for(int i = 0; i < propCount; i++) {
        if(propTypes[i] != GL_SAMPLER_2D) {
            continue;
        }
        glGetActiveUniformName(target->ProgramId, i, MAX_UNIFORM_NAME_LEN, 0, buffer);

        std::string uniformName = buffer;
        GLuint loc = glGetUniformLocation(target->ProgramId, uniformName.c_str());
        glUniform1i(loc, textures);
        target->textureIdMap.insert(std::make_pair(uniformName, textures));
        textures++;
    }
    delete[] propIndices;
    delete[] propTypes;

    target->state = Resource::Success;

    target->mvpLocation = target->getUniformId("mvp");
    target->modelMatrixLocation = target->getUniformId("modelMatrix");
}
