
#include "renderer/RenderableTexture.h"

#include "resources/RenderResources.h"

std::shared_ptr<Resource> RenderableTextureBuilder::construct()
{
    return std::make_shared<RenderableTexture>();
}

void RenderableTextureBuilder::init()
{
    addDependency(sourceTexture);
}

void RenderableTextureBuilder::startBuild()
{
    std::shared_ptr<RenderableTexture> outTexture = getResource<RenderableTexture>();
    std::shared_ptr<Texture> texture = getDependency<Texture>(sourceTexture, (uint)RenderResources::Texture);
    if(!texture || texture->getMode() == Texture::INVALID) {
        throw "Bad Texture!";
    }

    GLuint texId;
    glCreateTextures(GL_TEXTURE_2D, 1, &outTexture->textureId);
    texId = outTexture->textureId;
    uint filter;
    if(mipMapLevels > 1) { filter = 0x2700 | minFilter | (minFilterMipMap << 1); }
    else { filter = (minFilter == Linear ? GL_LINEAR : GL_NEAREST); }
    glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, magFilter == Linear ? GL_LINEAR : GL_NEAREST);

    glTextureParameteri(texId, GL_TEXTURE_WRAP_S, wrapU == Clamp ? GL_CLAMP : GL_REPEAT);
    glTextureParameteri(texId, GL_TEXTURE_WRAP_T, wrapV == Clamp ? GL_CLAMP : GL_REPEAT);

    if(texture->getMode() == Texture::RGB_8) {
        glTextureStorage2D(texId, mipMapLevels, GL_RGB8, texture->getWidth(), texture->getHeight());
        glTextureSubImage2D(texId, 0, 0, 0, texture->getWidth(), texture->getHeight(),
            GL_RGB, GL_UNSIGNED_BYTE, texture->asRGB_8());
        glGenerateTextureMipmap(texId);
    } else if(texture->getMode() == Texture::RGBA_8) {
        glTextureStorage2D(texId, mipMapLevels, GL_RGBA8, texture->getWidth(), texture->getHeight());
        glTextureSubImage2D(texId, 0, 0, 0, texture->getWidth(), texture->getHeight(),
            GL_RGBA, GL_UNSIGNED_BYTE, texture->asRGBA_8());
        glGenerateTextureMipmap(texId);
    }

    outTexture->state = Resource::Success;
}
