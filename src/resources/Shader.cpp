
#include "resources/Shader.h"

#include "utility/Serializer.h"

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

void Shader::loadFromFile(std::ifstream& file)
{
    Serializer ser(&file);
    read(ser, *this);
}

void Shader::saveToFile(std::ofstream& file)
{
    Serializer ser(&file);
    write(ser, *this);
}
