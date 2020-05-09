
#pragma once

#include "std.h"

#include "resources/ResourceLoader.h"
#include "utility/Serializer.h"

#include "Colour.h"

class Texture : public Resource
{
public:
    enum Mode : uchar {
        INVALID,
        RGB_8,
        RGBA_8
    };

    Texture();
    ~Texture();

    void fromColour3(Colour3* _data, uint _width, uint _height);
    void fromColour4(Colour4* _data, uint _width, uint _height);

    void cleanUp();

    inline uint getWidth() const { return width; }
    inline uint getHeight() const { return height; }
    inline Mode getMode() const { return mode; }
    inline Colour3* asRGB_8() const { return data.rgb_8; }
    inline Colour4* asRGBA_8() const { return data.rgba_8; }
private:
    Mode mode = INVALID;
    uint width;
    uint height;
    union {
        Colour3* rgb_8;
        Colour4* rgba_8;
    } data;
};

template<>
void write(Serializer& ser, const Texture& texture);

template<>
void read(Serializer& ser, Texture& texture);

void writeTexture(Serializer& ser, void* textureRaw);

void* readTexture(Serializer& ser);

void readIntoTexture(Serializer& ser, void* textureRaw);
