
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
        vec2 scaledCharSize;
        vec2 scaledCharBearing;
        float advance;

        operator bool() const { return valid; }
    };

    struct CharacterLayout
    {
        vec4 textureLayout;
        vec4 physicalLayout;
    };

    struct StringLayout
    {
        vector<CharacterLayout> layout;
        vec2 bounds;
    };

    StringLayout layoutString(const string& text, float desiredFontSize,
        float width, float lineSpacing = 1) const;

    StringLayout layoutStringUnbounded(const string& text, float desiredFontSize, float lineSpacing = 1) const;

    shared_ptr<RenderableTexture> getTextureSheet() const { return texture; }

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
    float spaceAdvance;
    float lineHeight;
    float maxDescent;

    shared_ptr<RenderableTexture> texture;
};
