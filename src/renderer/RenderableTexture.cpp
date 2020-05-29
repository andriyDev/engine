
#include "renderer/RenderableTexture.h"

#include "resources/RenderResources.h"

RenderableTexture::RenderableTexture(ResourceRef<Texture> sourceTexture, WrapMode wrapU, WrapMode wrapV,
    FilterMode minFilter, FilterMode minFilterMipMap, FilterMode magFilter, uint mipMapLevels)
    : sourceTextureRef(sourceTexture)
{
    std::shared_ptr<BuildData> data = std::make_shared<BuildData>();
    data->wrapU = wrapU;
    data->wrapV = wrapV;
    data->minFilter = minFilter;
    data->minFilterMipMap = minFilterMipMap;
    data->magFilter = magFilter;
    data->mipMapLevels = mipMapLevels;
    resolveDependencies(Immediate);
    if(!load(data)) {
        throw "Failed to create RenderableTexture";
    }
}

RenderableTexture::~RenderableTexture()
{
    if(textureId) {
        glDeleteTextures(1, &textureId);
    }
}

void RenderableTexture::bind(GLuint textureUnit)
{
    glBindTextureUnit(textureUnit, textureId);
}

bool RenderableTexture::load(std::shared_ptr<void> data)
{
    std::shared_ptr<BuildData> bd = std::dynamic_pointer_cast<BuildData>(data);
    std::shared_ptr<Texture> texture = sourceTextureRef.resolve(Immediate); // Make sure this is loaded.
    if(!texture || texture->getMode() == Texture::INVALID) {
        throw "Bad Texture!";
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
    uint filter;
    if(bd->mipMapLevels > 1) { filter = 0x2700 | bd->minFilter | (bd->minFilterMipMap << 1); }
    else { filter = (bd->minFilter == Linear ? GL_LINEAR : GL_NEAREST); }
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, bd->magFilter == Linear ? GL_LINEAR : GL_NEAREST);

    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, bd->wrapU == Clamp ? GL_CLAMP : GL_REPEAT);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, bd->wrapV == Clamp ? GL_CLAMP : GL_REPEAT);

    if(texture->getMode() == Texture::RGB_8) {
        glTextureStorage2D(textureId, bd->mipMapLevels, GL_RGB8, texture->getWidth(), texture->getHeight());
        glTextureSubImage2D(textureId, 0, 0, 0, texture->getWidth(), texture->getHeight(),
            GL_RGB, GL_UNSIGNED_BYTE, texture->asRGB_8());
        glGenerateTextureMipmap(textureId);
    } else if(texture->getMode() == Texture::RGBA_8) {
        glTextureStorage2D(textureId, bd->mipMapLevels, GL_RGBA8, texture->getWidth(), texture->getHeight());
        glTextureSubImage2D(textureId, 0, 0, 0, texture->getWidth(), texture->getHeight(),
            GL_RGBA, GL_UNSIGNED_BYTE, texture->asRGBA_8());
        glGenerateTextureMipmap(textureId);
    }

    sourceTextureRef = ResourceRef<Texture>();
    return true;
}

std::shared_ptr<RenderableTexture> RenderableTexture::build(std::shared_ptr<BuildData> data)
{
    std::shared_ptr<RenderableTexture> texture = std::make_shared<RenderableTexture>();
    texture->sourceTextureRef = data->sourceTexture;
    return texture;
}
