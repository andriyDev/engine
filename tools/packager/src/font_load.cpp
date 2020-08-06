
#include "std.h"
#include "font/Font.h"
#include <ft2build.h>
#include <freetype/freetype.h>

#define FONT_CHAR_ROW 10

pair<Font*, Texture*> loadFont(string fontFile, uint faceIndex, uint size)
{
    FT_Library freeType;
    FT_Face face;

    if(FT_Init_FreeType(&freeType)) {
        cerr << "Failed to load FreeType" << endl;
        return make_pair(nullptr, nullptr);
    }

    if(FT_New_Face(freeType, fontFile.c_str(), faceIndex, &face)) {
        cerr << "Failed to load FontFace from '" << fontFile << "'" << endl;
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }

    Font* font = new Font();

    FT_Set_Pixel_Sizes(face, 0, size);

    uvec2 currentOffset(0,0);
    uvec2 texDimensions(0,0);

    int orderIndex = 0;

    font->lineHeight = (ushort)(face->size->metrics.height >> 6);
    font->sourceFontSize = (ushort)size;

    if(FT_Load_Char(face, 32, FT_LOAD_ADVANCE_ONLY)) {
        fprintf(stderr, "Could not load space info during Font loading.\n");

        delete font;
        FT_Done_Face(face);
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }
    font->spaceAdvance = (uchar)(face->glyph->advance.x >> 6);

    font->maxAscent = 0;
    font->maxDescent = 0;

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_BITMAP_METRICS_ONLY | FT_LOAD_ADVANCE_ONLY)) {
            font->characters[c].valid = false;
        } else {
            font->characters[c].valid = true;
            // The position in the texture is the current offset.
            font->characters[c].pos = currentOffset + uvec2(1,1); // + 1 for margin.
            font->characters[c].size = uvec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            font->characters[c].bearing = ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            font->characters[c].advance = (ushort)(face->glyph->advance.x >> 6);
            // Shift the offset to the right. This will be the new right-most edge for this row.
            currentOffset.x += font->characters[c].size.x + 2; // + 2 for margin.
            // Take whichever is more right, the current width, or the right-most edge of this glyph.
            texDimensions.x = glm::max(currentOffset.x, texDimensions.x);
            // Take whichever is more down, the current height, or the bottom edge of this glyph.
            texDimensions.y = glm::max(currentOffset.y + font->characters[c].size.y + 2, texDimensions.y);

            font->maxAscent = (ushort)glm::max((int)font->maxAscent, font->characters[c].bearing.y);
            font->maxDescent = (ushort)glm::max((int)font->maxDescent,
                (int)font->characters[c].size.y - (int)font->characters[c].bearing.y);
            // Increment the order index.
            orderIndex++;
            // If we've started a new row, shift to the left, and below the current height.
            if(orderIndex % FONT_CHAR_ROW == 0) {
                currentOffset.x = 0;
                currentOffset.y = texDimensions.y;
            }
        }
    }

    if(texDimensions.x == 0 || texDimensions.y == 0) {
        fprintf(stderr, "Something went horribly wrong with Font loading.\n");
        delete font;
        FT_Done_Face(face);
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }

    font->sourceSize = uvec2(texDimensions.x, texDimensions.y);
    uchar* textureData = new uchar[texDimensions.x * texDimensions.y];

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(!font->characters[c].valid) {
            continue;
        }

        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_RENDER)) {
            font->characters[c].valid = false;
            continue;
        }

        uvec2 start = font->characters[c].pos;
        uvec2 end = start + font->characters[c].size;

        // Clear out the top margin.
        for(uint x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + (start.y - 1) * texDimensions.x] = 0;
        }
        for(uint y = start.y; y < end.y; y++) {
            // Clear out left margin.
            textureData[start.x - 1 + y * texDimensions.x] = 0;
            // Fill in this row of the texture with the bitmap.
            for(uint x = start.x; x < end.x; x++) {
                uchar cc = face->glyph->bitmap.buffer[
                    (x - start.x) + (y - start.y) * font->characters[c].size.x];
                textureData[x + y * texDimensions.x] = cc;
            }
            textureData[end.x + y * texDimensions.x] = 0;
        }
        // Clear out the bottom margin.
        for(uint x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + end.y * texDimensions.x] = 0;
        }
    }

    // Build the texture.
    Texture* texture = new Texture();
    texture->fromGreyscale(textureData, texDimensions.x, texDimensions.y);

    FT_Done_Face(face);
    FT_Done_FreeType(freeType);

    return make_pair(font, texture);
}
