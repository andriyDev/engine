
#include "font/Font.h"

#include "resources/Texture.h"

shared_ptr<Resource::BuildData> Font::createAssetData(uint texture, string fileName)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->texture = texture;
    data->fileName = fileName;
    return data;
}

shared_ptr<Resource> Font::build(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Font::BuildData> d = static_pointer_cast<Font::BuildData>(data);
    shared_ptr<Font> font(new Font());
    font->texture = d->texture;
    return font;
}

Font::Font(ResourceRef<RenderableTexture> _texture, string fileName)
    : texture(_texture)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->fileName = fileName;
    resolveDependencies(Immediate);
    if(!load(data)) {
        throw "Failed to load Font";
    }
}

bool Font::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Font::BuildData> d = static_pointer_cast<Font::BuildData>(data);
    ifstream file(d->fileName, ios_base::binary | ios_base::in);
    if(!file.is_open()) {
        return false;
    }

    sourceFontSize = read_ushort(&file);
    sourceSize.x = read_ushort(&file);
    sourceSize.y = read_ushort(&file);
    spaceAdvance = read_ushort(&file);
    lineHeight = read_ushort(&file);
    maxDescent = read_ushort(&file);
    uchar packed_bool = 0;
    for(uint i = 0; i < FONT_CHAR_COUNT; i++) {
        if((i & 0b00000111) == 0) {
            packed_bool = read_uchar(&file);
        }
        characters[i].valid = packed_bool & 0b10000000;
        packed_bool <<= 1;
    }
    for(uint i = 0; i < FONT_CHAR_COUNT; i++) {
        if(!characters[i]) {
            continue;
        }

        characters[i].pos.x = read_ushort(&file);
        characters[i].pos.y = read_ushort(&file);
        characters[i].size.x = read_ushort(&file);
        characters[i].size.y = read_ushort(&file);
        characters[i].bearing.x = read_short(&file);
        characters[i].bearing.y = read_short(&file);
        characters[i].advance = read_ushort(&file);
    }

    file.close();
    return true;
}

bool Font::save(string fileName)
{
    ofstream file(fileName, ios_base::binary | ios_base::out);
    if(!file.is_open()) {
        return false;
    }
    
    write_ushort(&file, sourceFontSize);
    write_ushort(&file, (ushort)sourceSize.x);
    write_ushort(&file, (ushort)sourceSize.y);
    write_ushort(&file, spaceAdvance);
    write_ushort(&file, lineHeight);
    write_ushort(&file, maxDescent);
    uchar packed_bool = 0;
    for(uint i = 0; i < FONT_CHAR_COUNT; i++) {
        packed_bool = (packed_bool << 1) | (uchar)characters[i].valid;
        // If we just packed the 8th bit (we're about to roll over), write the byte.
        if((i & 0b00000111) == 7) {
            write_uchar(&file, packed_bool);
        }
    }
    // If there are left over bytes, write them out.
    if((FONT_CHAR_COUNT & 0b00000111) != 0) {
        // Shift it over so that the valid bits are to the left of the byte.
        packed_bool = packed_bool << (8 - (FONT_CHAR_COUNT & 0b00000111));
        write_uchar(&file, packed_bool);
    }
    for(uint i = 0; i < FONT_CHAR_COUNT; i++) {
        if(!characters[i]) {
            continue;
        }

        write_ushort(&file, (ushort)characters[i].pos.x);
        write_ushort(&file, (ushort)characters[i].pos.y);
        write_ushort(&file, (ushort)characters[i].size.x);
        write_ushort(&file, (ushort)characters[i].size.y);
        write_short(&file, (short)characters[i].bearing.x);
        write_short(&file, (short)characters[i].bearing.y);
        write_ushort(&file, characters[i].advance);
    }

    file.close();
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

vector<Token> tokenize(const string& text, const Font::Character* characters, ushort spaceAdvance, float pixelUnit)
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
            currentToken.width += (c == '\t' ? 4 : 1) * (spaceAdvance * pixelUnit);
        } else {
            currentToken.width += characters[c - FONT_CHAR_START].advance * pixelUnit;
        }
    }
    // A token was left over so put in the last one.
    if(currentToken) {
        tokens.push_back(currentToken);
    }
    return tokens;
}

int splitToken(const vector<float>& widths, int offset, float remainingLineSpace, float fullLineSpace)
{
    // If the next character can't fit here, but it can fit on the next line, just move to the next line.
    if(widths[offset] > remainingLineSpace && widths[offset] <= fullLineSpace) {
        return offset;
    }
    // We already know the next character must be taken.
    float sum = widths[offset];
    // Shift over once.
    offset++;
    for(; offset < widths.size(); offset++) {
        sum += widths[offset];
        // If adding the next character pushes us to the next line, 
        if(sum > remainingLineSpace) {
            return offset;
        }
    }
    return offset;
}

void newLine(vec2& offset, Font::StringLayout& layout, ushort lineHeight, float pixelUnit, float lineSpacing,
    int lines = 1)
{
    offset.x = 0;
    offset.y += lineHeight * pixelUnit * lineSpacing * lines;
    layout.bounds = vec2(layout.bounds.x, max(layout.bounds.y, offset.y));
}

Font::StringLayout Font::layoutString(const string& text, float desiredFontSize, float width, float lineSpacing) const
{
    StringLayout layoutData;
    float pixelUnit = desiredFontSize / sourceFontSize;

    vector<Token> tokens = tokenize(text, characters, spaceAdvance, pixelUnit);

    vec2 offset(0, lineHeight * pixelUnit);
    layoutData.bounds = offset;

    for(Token& token : tokens) {
        uchar type = getCharType(text[token.start]);
        if(type == NEWLINE) {
            newLine(offset, layoutData, lineHeight, pixelUnit, lineSpacing, token.length);
            continue;
        }

        vector<float> widths;
        if(type == WHITESPACE) {
            // Go through each character and add its width.
            for(int i = 0; i < token.length; i++) {
                widths.push_back((text[token.start + i] == '\t' ? 4 : 1) * (spaceAdvance * pixelUnit));
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

        int lastChar = 0;
        int currentChar = 0;
        // Keep going until offset is done the string.
        while(currentChar != widths.size()) {
            // If the token will overflow to the next line, we should just move directly to the next line.
            if(token.width > width - offset.x && token.width <= width) {
                newLine(offset, layoutData, lineHeight, pixelUnit, lineSpacing);
                currentChar = (int)widths.size();
            } else {
                // For all iterations except the first, move to the next line.
                if(currentChar != 0) {
                    newLine(offset, layoutData, lineHeight, pixelUnit, lineSpacing);
                }
                
                // Split the token.
                currentChar = splitToken(widths, lastChar, width - offset.x, width);
                // There is a special case with splitToken if the first character will not fit on the current line,
                // Since we previously only move to the next line if not on the first iteration,
                // we have to handle this case.
                if(currentChar == 0) {
                    newLine(offset, layoutData, lineHeight, pixelUnit, lineSpacing);
                }
            }

            // Move lastOffset up until it matches with offset.
            for(; lastChar < currentChar; lastChar++) {
                // Only output characters if not whitespace.
                if(type != WHITESPACE) {
                    CharacterLayout charLayout;
                    const Character& info = characters[text[token.start + lastChar] - FONT_CHAR_START];
                    charLayout.textureLayout = vec4(vec2(info.pos) / vec2(sourceSize), vec2(info.size) / vec2(sourceSize));
                    vec2 tl = offset + vec2(info.bearing) * pixelUnit * vec2(1, -1);
                    charLayout.physicalLayout = vec4(tl, tl + vec2(info.size) * pixelUnit);
                    vec2 s = vec2(info.size) * pixelUnit;
                    layoutData.layout.push_back(charLayout);
                }
                offset.x += widths[lastChar];
                token.width -= widths[lastChar];
            }
            // We only need to update the bounds once this token chunk has been laid out.
            layoutData.bounds = vec2(max(layoutData.bounds.x, offset.x), layoutData.bounds.y);
        }
    }
    layoutData.bounds.y += maxDescent * pixelUnit;
    return layoutData;
}

Font::StringLayout Font::layoutStringUnbounded(const string& text, float desiredFontSize, float lineSpacing) const
{
    StringLayout layoutData;
    float pixelUnit = desiredFontSize / sourceFontSize;

    vector<Token> tokens = tokenize(text, characters, spaceAdvance, pixelUnit);

    vec2 offset(0, lineHeight * pixelUnit);
    layoutData.bounds = offset;

    for(Token& token : tokens) {
        uchar type = getCharType(text[token.start]);
        if(type == NEWLINE) {
            newLine(offset, layoutData, lineHeight, pixelUnit, lineSpacing, token.length);
            continue;
        }

        vector<float> widths;
        if(type == WHITESPACE) {
            // Go through each character and add its width.
            for(int i = 0; i < token.length; i++) {
                widths.push_back((text[token.start + i] == '\t' ? 4 : 1) * (spaceAdvance * pixelUnit));
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
        
        // Move lastOffset up until it matches with offset.
        for(int i = 0; i < token.length; i++) {
            // Only output characters if not whitespace.
            if(type != WHITESPACE) {
                CharacterLayout charLayout;
                const Character& info = characters[text[token.start + i] - FONT_CHAR_START];
                charLayout.textureLayout = vec4(vec2(info.pos) / vec2(sourceSize), vec2(info.size) / vec2(sourceSize));
                vec2 tl = offset + vec2(info.bearing) * pixelUnit * vec2(1, -1);
                charLayout.physicalLayout = vec4(tl, tl + vec2(info.size) * pixelUnit);
                layoutData.layout.push_back(charLayout);
            }
            offset.x += widths[i];
        }
        layoutData.bounds = vec2(max(layoutData.bounds.x, offset.x), layoutData.bounds.y);
    }
    layoutData.bounds.y += maxDescent * pixelUnit;
    return layoutData;
}
