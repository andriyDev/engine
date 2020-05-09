
#pragma once

#include "std.h"
#include "utility/Serializer.h"
#include "ResourceLoader.h"
#include "FileResourceBuilder.h"
#include "RenderResources.h"

class Shader : public Resource
{
public:
    Shader();

    std::string code;
};

class ShaderBuilder : public FileResourceBuilder<Shader>
{
public:
    ShaderBuilder(std::string resourceName, std::shared_ptr<PackageFile> resourcePackage)
    : FileResourceBuilder((uint)RenderResources::Shader, resourcePackage, resourceName,
        (uint)FileRenderResources::Shader) {}
};

template<>
void write(Serializer& ser, const Shader& shader);

template<>
void read(Serializer& ser, Shader& shader);

void writeShader(Serializer& ser, void* shaderRaw);

void* readShader(Serializer& ser);

void readIntoShader(Serializer& ser, void* shaderRaw);
