
#include "resources/Shader.h"
#include <sstream>

void Shader::loadFromFile(ifstream& file)
{
    file.seekg(0, ios::end);
    code.resize(file.tellg());
    file.seekg(0, ios::beg);

    file.read(&code[0], code.capacity());
}

void Shader::saveToFile(ofstream& file)
{
    file << code;
}
