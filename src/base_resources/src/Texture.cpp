
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

void Texture::loadFromFile(ifstream& file)
{
    uint width = read_uint(&file);
    uint height = read_uint(&file);
    uchar mode = read_uchar(&file);
    uint size = width * height;
    if(mode == Texture::RGB_8) {
        Colour3* data_loaded = new Colour3[size];
        for(uint i = 0; i < size; i++) {
            read_Colour3_inplace(&file, data_loaded + i);
        }
        fromColour3(data_loaded, width, height);
    } else if(mode == Texture::RGBA_8) {
        Colour4* data_loaded = new Colour4[size];
        for(uint i = 0; i < size; i++) {
            read_Colour4_inplace(&file, data_loaded + i);
        }
        fromColour4(data_loaded, width, height);
    } else if(mode == Texture::GREYSCALE_8) {
        uchar* data_loaded = new uchar[size];
        for(uint i = 0; i < size; i++) {
            data_loaded[i] = read_uchar(&file);
        }
        fromGreyscale(data_loaded, width, height);
    } else {
        throw "Invalid mode";
    }
}

void Texture::saveToFile(ofstream& file)
{
    if(getMode() == Texture::INVALID) {
        throw "Cannot write invalid texture!";
    }
    write_uint(&file, getWidth());
    write_uint(&file, getHeight());
    write_uchar(&file, (uchar)getMode());
    uint size = getWidth() * getHeight();
    if(getMode() == Texture::RGB_8) {
        for(uint i = 0; i < size; i++) {
            write_Colour3(&file, asRGB_8()[i]);
        }
    } else if(getMode() == Texture::RGBA_8) {
        for(uint i = 0; i < size; i++) {
            write_Colour4(&file, asRGBA_8()[i]);
        }
    } else {
        for(uint i = 0; i < size; i++) {
            write_uchar(&file, asGreyscale_8()[i]);
        }
    }
}
