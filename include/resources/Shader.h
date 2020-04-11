
#pragma once

#include "std.h"
#include "utility/Serializer.h"

#define RESOURCE_SHADER 2

class Shader
{
public:
    string code;
};

template<>
void write(Serializer& ser, const Shader& shader);

template<>
void read(Serializer& ser, Shader& shader);

void writeShader(Serializer& ser, void* shaderRaw);

void* readShader(Serializer& ser);
