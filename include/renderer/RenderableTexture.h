
#pragma once

#include "std.h"
#include "resources/Texture.h"
#include "resources/ResourceLoader.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableTexture : public Resource
{
public:
    RenderableTexture();
    ~RenderableTexture();

    void bind(GLuint textureUnit);

protected:
    GLuint textureId;

    friend class RenderableTextureBuilder;
};

class RenderableTextureBuilder : public ResourceBuilder
{
public:
    enum WrapMode : uchar {
        Repeat,
        Clamp
    };

    enum FilterMode : uchar {
        Nearest,
        Linear
    };

    RenderableTextureBuilder();

    WrapMode wrapU = Repeat;
    WrapMode wrapV = Repeat;

    uint mipMapLevels = 1;

    FilterMode minFilter = Nearest;
    FilterMode minFilterMipMap = Linear;
    FilterMode magFilter = Linear;

    std::string sourceTexture;

    virtual std::shared_ptr<Resource> construct() override;

    virtual void init() override;

    virtual void startBuild() override;
};