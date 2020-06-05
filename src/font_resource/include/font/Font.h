
#pragma once

#include "std.h"

#include "resources/ResourceLoader.h"
#include "font/FontFace.h"
#include "renderer/RenderableTexture.h"

#define FONT_CHAR_START 33
#define FONT_CHAR_END 127
#define FONT_CHAR_COUNT FONT_CHAR_END - FONT_CHAR_START

class Font : public Resource
{
public:
    Font(ResourceRef<FontFace> _fontFace, uint size);

    static shared_ptr<BuildData> createAssetData(uint font, uint size);

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data);

    struct Character
    {
        bool valid;
        vec2 textureStart;
        vec2 textureSize;
        ivec2 charPos;
        ivec2 charSize;
        ivec2 charBearing;
        unsigned int advance;

        operator bool() const { return valid; }
    };

    struct CharacterLayout
    {
        vec2 textureStart;
        vec2 textureSize;
        vec2 shift;
        vec2 size;
    };

protected:
    class BuildData : public Resource::BuildData
    {
    public:
        uint font; // Font Id.
        uint size; // In pixels
    };

    Font() {}

    virtual vector<uint> getDependencies() override { return { fontFace }; }
    virtual void resolveDependencies(ResolveMethod method) override { fontFace.resolve(method); }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;
    
    ResourceRef<FontFace> fontFace;

    Character characters[FONT_CHAR_COUNT];

    shared_ptr<RenderableTexture> texture;
};
