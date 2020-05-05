
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

GLuint MaterialProgram::getUniformId(const std::string& uniformName) const
{
    return state == Success ? glGetUniformLocation(ProgramId, uniformName.c_str()) : 0;
}

GLuint MaterialProgram::getProgramId() const
{
    return ProgramId;
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

    target->state = Resource::Success;
}
