
#include "renderer/MaterialProgram.h"

void compileShader(GLuint shaderId, const vector<Shader*>& components)
{
    GLint Result = GL_FALSE;
    int infoLength;

    vector<const char*> rawStrings;
    rawStrings.reserve(components.size());
    for(Shader* shaderComp : components) {
        rawStrings.push_back(check(shaderComp)->code.c_str());
    }
    glShaderSource(shaderId, (GLsizei)rawStrings.size(), &rawStrings[0], NULL);
    glCompileShader(shaderId);

    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLength);
    if(infoLength > 0) {
        vector<char> errorMsg(infoLength + 1);
        glGetShaderInfoLog(shaderId, infoLength, NULL, &errorMsg[0]);
        fprintf(stderr, "Error (Shader Compilation):\n%s\n", &errorMsg[0]);
        throw 1;
    }
}

MaterialProgram::MaterialProgram(
    const vector<Shader*>& vertexShaderComponents,
    const vector<Shader*>& fragmentShaderComponents)
{
    assert(!vertexShaderComponents.empty());
    assert(!fragmentShaderComponents.empty());

    GLuint shaders[2];
    shaders[0] = glCreateShader(GL_VERTEX_SHADER);
    shaders[1] = glCreateShader(GL_FRAGMENT_SHADER);

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
        vector<char> errorMsg(infoLength + 1);
        glGetProgramInfoLog(ProgramId, infoLength, NULL, &errorMsg[0]);
        fprintf(stderr, "Error (Program Linking):\n%s\n", &errorMsg[0]);
        throw 1;
    }

    glDetachShader(ProgramId, shaders[0]);
    glDetachShader(ProgramId, shaders[1]);
    
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);
}

MaterialProgram::~MaterialProgram()
{
    glDeleteProgram(ProgramId);
}

void MaterialProgram::bind()
{
    glUseProgram(ProgramId);
}
