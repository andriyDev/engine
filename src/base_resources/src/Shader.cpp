
#include "resources/Shader.h"
#include <sstream>

void Shader::loadFromFile(std::ifstream& file)
{
    file.seekg(0, std::ios::end);
    code.resize(file.tellg());
    file.seekg(0, std::ios::beg);

    file.read(&code[0], code.capacity());
}

void Shader::saveToFile(std::ofstream& file)
{
    file << code;
}
