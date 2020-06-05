
#include "resources/Texture.h"

#include "utility/Serializer.h"

Texture::~Texture()
{
    cleanUp();
}

void Texture::fromGreyscale(uchar* _data, uint _width, uint _height)
{
    cleanUp();

    data.greyscale_8 = _data;
    width = _width;
    height = _height;
    mode = GREYSCALE_8;
}

void Texture::fromColour3(Colour3* _data, uint _width, uint _height)
{
    cleanUp();

    data.rgb_8 = _data;
    width = _width;
    height = _height;
    mode = RGB_8;
}

void Texture::fromColour4(Colour4* _data, uint _width, uint _height)
{
    cleanUp();

    data.rgba_8 = _data;
    width = _width;
    height = _height;
    mode = RGBA_8;
}

void Texture::cleanUp()
{
    if(mode == RGB_8) {
        delete[] data.rgb_8;
    } else if(mode == RGBA_8) {
        delete[] data.rgba_8;
    } else if(mode == GREYSCALE_8) {
        delete[] data.greyscale_8;
    }
}

template<>
void write(Serializer& ser, const Texture& texture)
{
    if(texture.getMode() == Texture::INVALID) {
        throw "Cannot write invalid texture!";
    }
    write(ser, texture.getWidth());
    write(ser, texture.getHeight());
    write(ser, (uchar)texture.getMode());
    if(texture.getMode() == Texture::RGB_8) {
        ser.write_raw((char*)texture.asRGB_8(), texture.getWidth() * texture.getHeight() * sizeof(Colour3));
    } else if(texture.getMode() == Texture::RGBA_8) {
        ser.write_raw((char*)texture.asRGBA_8(), texture.getWidth() * texture.getHeight() * sizeof(Colour4));
    } else if(texture.getMode() == Texture::GREYSCALE_8) {
        ser.write_raw((char*)texture.asGreyscale_8(), texture.getWidth() * texture.getHeight() * sizeof(uchar));
    }
}

template<>
void read(Serializer& ser, Texture& texture)
{
    uint w, h;
    read(ser, w);
    read(ser, h);
    uchar modeRaw;
    read(ser, modeRaw);
    Texture::Mode mode = (Texture::Mode)modeRaw;
    if(mode == Texture::RGB_8) {
        Colour3* data = new Colour3[w * h];
        ser.read_raw((char*)data, w * h * sizeof(Colour3));
        texture.fromColour3(data, w, h);
    } else if(mode == Texture::RGBA_8) {
        Colour4* data = new Colour4[w * h];
        ser.read_raw((char*)data, w * h * sizeof(Colour4));
        texture.fromColour4(data, w, h);
    } else if(mode == Texture::GREYSCALE_8) {
        uchar* data = new uchar[w * h];
        ser.read_raw((char*)data, w * h * sizeof(uchar));
        texture.fromGreyscale(data, w, h);
    } else {
        throw "Invalid mode";
    }
}

void Texture::loadFromFile(ifstream& file)
{
    Serializer ser(&file);
    read(ser, *this);
}

void Texture::saveToFile(ofstream& file)
{
    Serializer ser(&file);
    write(ser, *this);
}
