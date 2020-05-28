
#pragma once

#include "std.h"
#include "resources/Texture.h"
#include "resources/ResourceLoader.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableTexture : public Resource
{
public:
    virtual ~RenderableTexture();

    void bind(GLuint textureUnit);

    static std::shared_ptr<Resource> build(std::shared_ptr<void> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

protected:
    GLuint textureId = 0;

    ResourceRef<Texture> sourceTextureRef;

    virtual std::vector<uint> getDependencies() override {
        return { sourceTextureRef };
    }
    virtual void resolveDependencies() override {
        sourceTextureRef.resolve();
    }
    virtual bool load(std::shared_ptr<void> data) override;

private:
    struct BuildData
    {
        enum WrapMode : uchar {
            Repeat,
            Clamp
        };

        enum FilterMode : uchar {
            Nearest,
            Linear
        };

        uint sourceTexture;

        WrapMode wrapU = Repeat;
        WrapMode wrapV = Repeat;

        uint mipMapLevels = 1;

        FilterMode minFilter = Nearest;
        FilterMode minFilterMipMap = Linear;
        FilterMode magFilter = Linear;
    };

    static std::shared_ptr<RenderableTexture> build(std::shared_ptr<BuildData> data);
};
