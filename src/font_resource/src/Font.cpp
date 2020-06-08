
#include "font/Font.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#include "resources/Texture.h"

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

    lineHeight = (float)d->size * face->height / face->units_per_EM;

    if(FT_Load_Char(face, 32, FT_LOAD_ADVANCE_ONLY)) {
        fprintf(stderr, "Could not load space info during Font loading.\n");
        return false;
    }
    spaceAdvance = (float)(face->glyph->advance.x >> 6);

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_BITMAP_METRICS_ONLY | FT_LOAD_ADVANCE_ONLY)) {
            characters[c].valid = false;
        } else {
            characters[c].valid = true;
            // The position in the texture is the current offset.
            characters[c].charPos = currentOffset + ivec2(1,1); // + 1 for margin.
            characters[c].charSize = ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            characters[c].charBearing = ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            characters[c].advance = (float)(face->glyph->advance.x >> 6);
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

    if(texDimensions.x == 0 || texDimensions.y == 0) {
        fprintf(stderr, "Something went horribly wrong with Font loading.\n");
        return false;
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
        for(int y = start.y; y < end.y; y++) {
            // Clear out left margin.
            textureData[start.x - 1 + y * texDimensions.x] = 0;
            // Fill in this row of the texture with the bitmap.
            for(int x = start.x; x < end.x; x++) {
                uchar cc = face->glyph->bitmap.buffer[
                    (x - start.x) + (y - start.y) * characters[c].charSize.x];
                textureData[x + y * texDimensions.x] = cc;
            }
            textureData[end.x + y * texDimensions.x] = 0;
        }
        // Clear out the bottom margin.
        for(int x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + end.y * texDimensions.x] = 0;
        }
    }

    // Build the texture.
    shared_ptr<Texture> textureSrc = shared_ptr<Texture>(new Texture());
    textureSrc->fromGreyscale(textureData, texDimensions.x, texDimensions.y);

    // Convert it to a RenderableTexture which we will use.
    texture = shared_ptr<RenderableTexture>(new RenderableTexture(textureSrc,
        RenderableTexture::Clamp, RenderableTexture::Clamp,
        RenderableTexture::Linear, RenderableTexture::Linear, RenderableTexture::Linear, 1
    ));

    // Clear the font face reference, we no longer need it.
    fontFace = ResourceRef<FontFace>();
    return true;
}

inline bool isAlphabetic(char c)
{
    return 'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z';
}

inline bool isNumeric(char c)
{
    return '0' <= c && c <= '9';
}

inline bool isWhitespace(char c)
{
    return !(FONT_CHAR_START <= c && c < FONT_CHAR_END);
}

#define ALPHABETIC 1
#define NUMERIC 2
#define NEWLINE 3
#define WHITESPACE 4
#define SPECIAL 5

inline uchar getCharType(char c)
{
    if(isAlphabetic(c)) {
        return ALPHABETIC;
    } else if(isNumeric(c)) {
        return NUMERIC;
    } else if(c == '\n') {
        return NEWLINE;
    } else if(isWhitespace(c)) {
        return WHITESPACE;
    } else {
        return SPECIAL;
    }
}

struct Token
{
    int start;
    int length;
    float width;

    operator bool() const { return length; }
};

vector<Token> tokenize(const string& text, const Font::Character* characters, float spaceAdvance)
{
    vector<Token> tokens;

    uchar currentType = 0;
    Token currentToken = {0,0,0};
    for(const char& c : text) {
        // Figure out the character type.
        uchar type = getCharType(c);
        // Start a new token if the types don't match or its a special character.
        if(type == SPECIAL || type != currentType) {
            if(currentToken) {
                tokens.push_back(currentToken);
            }
            currentType = type;
            // Shift over by the length, and clear out the token.
            currentToken.start += currentToken.length;
            currentToken.length = 0;
            currentToken.width = 0;
        }
        // Add the character to the token.
        currentToken.length += 1;
        // Compute its width and add it to the tokens width.
        if(type == NEWLINE) {
            currentToken.width += 0;
        } else if(type == WHITESPACE) {
            currentToken.width += c == '\t' ? 4 * spaceAdvance : spaceAdvance;
        } else {
            currentToken.width += characters[c - FONT_CHAR_START].advance;
        }
    }
    // A token was left over so put in the last one.
    if(currentToken) {
        tokens.push_back(currentToken);
    }
    return tokens;
}

vector<pair<int, int>> splitTokenForLayout(const vector<float>& widths, float sumWidth,
    float remainingWidthFirstLine, float width)
{
    vector<pair<int, int>> splits;
    int i = 0;
    while(i < widths.size()) {
        if(sumWidth <= remainingWidthFirstLine) {
            splits.push_back(make_pair(i, (int)widths.size()));
            break;
        }
        else if(sumWidth <= width) {
            remainingWidthFirstLine = width;
            splits.push_back(make_pair(i, i));
        } else if(widths[i] > remainingWidthFirstLine) { // If the character won't fit on this line.
            if(widths[i] > width) {
                // Place this character anyway since it won't fit in the next line either.
                splits.push_back(make_pair(i, i + 1));
                ++i;
            } else { // Skip to the next line.
                splits.push_back(make_pair(i, i));
            }
            remainingWidthFirstLine = width;
        } else {
            float sumShift = widths[i]; // We already know the next character won't push us past the limit.
            // Skip the first character.
            for(int j = i + 1; j < widths.size(); j++) {
                // If the next character will tip us over the limit for the line.
                if(sumShift + widths[j] > remainingWidthFirstLine) {
                    splits.push_back(make_pair(i, j));
                    i = j;
                    break;
                }
                sumShift += widths[i];
            }
            // Since we've gone to the next line, we need to reset the remaining width.
            remainingWidthFirstLine = width;
        }
    }
    return splits;
}

vector<Font::CharacterLayout> Font::layoutString(const string& text, float pixelUnit, float width,
    float lineSpacing) const
{
    vector<CharacterLayout> layout;

    vector<Token> tokens = tokenize(text, characters, spaceAdvance);

    vec2 offset(0, lineHeight * lineSpacing * pixelUnit);

    for(Token& token : tokens) {
        uchar type = getCharType(text[token.start]);
        if(type == NEWLINE) {
            offset.x = 0;
            offset.y += lineHeight * lineSpacing * pixelUnit * token.length;
            continue;
        }

        vector<float> widths;
        if(type == WHITESPACE) {
            // Go through each character and add its width.
            for(int i = 0; i < token.length; i++) {
                widths.push_back((text[token.start + i] == '\t' ? 4 * spaceAdvance : spaceAdvance) * pixelUnit);
            }
        } else if(type == ALPHABETIC || type == NUMERIC) {
            // Go through each character and add its width.
            for(int i = 0; i < token.length; i++) {
                widths.push_back(characters[text[token.start + i] - FONT_CHAR_START].advance * pixelUnit);
            }
        } else if(type == SPECIAL) {
            // We know SPECIAL tokens only have one character so just lay it out.
            widths.push_back(characters[text[token.start] - FONT_CHAR_START].advance * pixelUnit);
        }

        vector<pair<int, int>> lineRanges = splitTokenForLayout(widths, token.width * pixelUnit,
            width - offset.x, width);
        int j = 0;
        for(const pair<int, int>& range : lineRanges) {
            for(int i = range.first; i < range.second; i++) {
                if(type != WHITESPACE) {
                    CharacterLayout charLayout;
                    const Character& info = characters[text[token.start + i] - FONT_CHAR_START];
                    charLayout.textureLayout = vec4(info.textureStart, info.textureSize);
                    vec2 tl = offset + vec2(info.charBearing) * pixelUnit * vec2(1, -1);
                    charLayout.physicalLayout = vec4(tl, tl + vec2(info.charSize) * pixelUnit);
                    layout.push_back(charLayout);
                }
                offset.x += widths[i];
            }
            if((++j) != lineRanges.size()) {
                offset.x = 0;
                offset.y += lineHeight * lineSpacing * pixelUnit;
            }
        }
    }
    return layout;
}
