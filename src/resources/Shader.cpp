
#include "resources/Shader.h"

template<>
void write(Serializer& ser, const Shader& shader)
{
    write_string<uint>(ser, shader.code);
}

template<>
void read(Serializer& ser, Shader& shader)
{
    read_string<uint>(ser, shader.code);
}

void writeShader(Serializer& ser, void* shaderRaw)
{
    Shader* shader = static_cast<Shader*>(shaderRaw);
    write(ser, *shader);
}

void* readShader(Serializer& ser)
{
    Shader* shader = new Shader();
    read(ser, *shader);
    return shader;
}
