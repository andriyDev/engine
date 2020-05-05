
#pragma once

#include "std.h"
#include "utility/Serializer.h"
#include "ResourceLoader.h"
#include "RenderResources.h"

class Shader : public Resource
{
public:
    Shader();

    std::string code;
};

template<>
void write(Serializer& ser, const Shader& shader);

template<>
void read(Serializer& ser, Shader& shader);

void writeShader(Serializer& ser, void* shaderRaw);

void* readShader(Serializer& ser);

void readIntoShader(Serializer& ser, void* shaderRaw);
