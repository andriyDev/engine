
#pragma once

#include "std.h"
#include "utility/Serializer.h"
#include "RenderResources.h"

class Shader
{
public:
    std::string code;
};

template<>
void write(Serializer& ser, const Shader& shader);

template<>
void read(Serializer& ser, Shader& shader);

void writeShader(Serializer& ser, void* shaderRaw);

void* readShader(Serializer& ser);
