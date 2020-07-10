
#pragma once

#include "std.h"

#include "resources/ResourceLoader.h"
#include "renderer/RenderableTexture.h"

#define FONT_CHAR_START 33
#define FONT_CHAR_END 127
#define FONT_CHAR_COUNT (FONT_CHAR_END - FONT_CHAR_START)

class Font : public Resource
{
public:
    Font(ResourceRef<RenderableTexture> _texture, string fileName);

    Font() {}

    static shared_ptr<BuildData> createAssetData(uint texture, string fileName);

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data);

    enum Alignment
    {
        Left,
        Right,
        Center
    };

    struct Character
    {
        bool valid;
        uvec2 pos;
        uvec2 size;
        ivec2 bearing;
        ushort advance;

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
        vector<vec2> advancePoints;
        vec2 bounds;
    };

    Character characters[FONT_CHAR_COUNT];
    uvec2 sourceSize;
    ushort sourceFontSize;
    ushort spaceAdvance;
    ushort lineHeight;
    ushort maxAscent;
    ushort maxDescent;

    StringLayout layoutString(const string& text, float desiredFontSize,
        float width, Alignment horizontalAlignment = Left, float lineSpacing = 1) const;

    StringLayout layoutStringUnbounded(const string& text, float desiredFontSize, float lineSpacing = 1) const;

    ResourceRef<RenderableTexture>& getTextureSheet() { return texture; }

    bool save(string fileName);

protected:
    class BuildData : public Resource::BuildData
    {
    public:
        uint texture; // Texture Id.
        string fileName; // The file to read font data from.
    };

    virtual vector<uint> getDependencies() override { return { texture }; }
    virtual void resolveDependencies(ResolveMethod method) override { texture.resolve(method); }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

    ResourceRef<RenderableTexture> texture;
};
