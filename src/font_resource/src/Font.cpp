
#include "font/Font.h"

#include <ft2build.h>
#include <freetype/freetype.h>

shared_ptr<Resource::BuildData> Font::createAssetData(uint font, uint size)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->font = font;
    data->size = size;
    return data;
}

shared_ptr<Resource> Font::build(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Font::BuildData> d = static_pointer_cast<Font::BuildData>(data);
    shared_ptr<Font> font(new Font());
    font->fontFace = d->font;
    return font;
}

Font::Font(ResourceRef<FontFace> _fontFace, uint size)
    : fontFace(_fontFace)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->size = size;
    resolveDependencies(Immediate);
    if(!load(data)) {
        throw "Failed to load FontFace";
    }
}

#define FONT_CHAR_ROW 10

bool Font::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Font::BuildData> d = static_pointer_cast<Font::BuildData>(data);
    shared_ptr<FontFace> fontFacePtr = fontFace.resolve(Immediate);
    FT_Face face = fontFacePtr->face_data;
    FT_Set_Pixel_Sizes(face, 0, d->size);

    ivec2 currentOffset(0,0);
    ivec2 texDimensions(0,0);

    int orderIndex = 0;

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_BITMAP_METRICS_ONLY)) {
            characters[c].valid = false;
        } else {
            characters[c].valid = true;
            // The position in the texture is the current offset.
            characters[c].charPos = currentOffset + ivec2(1,1); // + 1 for margin.
            characters[c].charSize = ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            characters[c].charBearing = ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            characters[c].advance = face->glyph->advance.x;
            // Shift the offset to the right. This will be the new right-most edge for this row.
            currentOffset.x += characters[c].charSize.x + 2; // + 2 for margin.
            // Take whichever is more right, the current width, or the right-most edge of this glyph.
            texDimensions.x = max<int>(currentOffset.x, texDimensions.x);
            // Take whichever is more down, the current height, or the bottom edge of this glyph.
            texDimensions.y = max<int>(currentOffset.y + characters[c].charSize.y + 2, texDimensions.y);
            // Increment the order index.
            orderIndex++;
            // If we've started a new row, shift to the left, and below the current height.
            if(orderIndex % FONT_CHAR_ROW == 0) {
                currentOffset.x = 0;
                currentOffset.y = texDimensions.y;
            }
        }
    }

    uchar* textureData = new uchar[texDimensions.x * texDimensions.y];

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(!characters[c].valid) {
            continue;
        }

        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_RENDER)) {
            characters[c].valid = false;
            continue;
        }

        ivec2 start = characters[c].charPos;
        ivec2 end = start + characters[c].charSize;

        // Compute the positions of the glyph in normalized texture coordinates.
        characters[c].textureStart = vec2(start) / vec2(texDimensions);
        characters[c].textureSize = vec2(characters[c].charSize) / vec2(texDimensions);

        // Clear out the top margin.
        for(int x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + (start.y - 1) * texDimensions.x] = 0;
        }
        for(int y = start.y; y < end.x; y++) {
            // Clear out left margin.
            textureData[start.x - 1 + y * texDimensions.x] = 0;
            // Fill in this row of the texture with the bitmap.
            for(int x = start.x; x < end.x; x++) {
                textureData[x + y * texDimensions.x] = face->glyph->bitmap.buffer[
                    (x - start.x) + (y - start.y) * characters[c].charSize.x];
            }
            textureData[end.x + 1 + y * texDimensions.x] = 0;
        }
        // Clear out the bottom margin.
        for(int x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + (end.y + 1) * texDimensions.x] = 0;
        }
    }

    // Build the actual texture.
    texture = shared_ptr<Texture>(new Texture());
    texture->fromGreyscale(textureData, texDimensions.x, texDimensions.y);

    // Clear the font face reference, we no longer need it.
    fontFace = ResourceRef<FontFace>();
    return true;
}
