
#pragma once

#include "std.h"
#include "resources/Texture.h"
#include "resources/ResourceLoader.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableTexture : public Resource
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

    RenderableTexture(ResourceRef<Texture> sourceTexture, WrapMode wrapU, WrapMode wrapV,
        FilterMode minFilter, FilterMode minFilterMipMap, FilterMode magFilter, uint mipMapLevels);
    virtual ~RenderableTexture();

    void bind(GLuint textureUnit);

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        uint sourceTexture;

        WrapMode wrapU = Repeat;
        WrapMode wrapV = Repeat;

        uint mipMapLevels = 1;

        FilterMode minFilter = Nearest;
        FilterMode minFilterMipMap = Linear;
        FilterMode magFilter = Linear;
    };

    static shared_ptr<BuildData> createAssetData(uint sourceTexture);

protected:
    RenderableTexture() {}

    GLuint textureId = 0;

    float width;
    float height;
    ResourceRef<Texture> sourceTextureRef;

    virtual vector<uint> getDependencies() override {
        return { sourceTextureRef };
    }
    virtual void resolveDependencies(ResolveMethod method) override {
        sourceTextureRef.resolve(method);
    }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

private:

    static shared_ptr<RenderableTexture> build(shared_ptr<BuildData> data);
};
