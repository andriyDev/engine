
#pragma once

#include "std.h"

#include "resources/FileResource.h"

#include "Colour.h"

class Texture : public FileResource
{
public:
    enum Mode : uchar {
        INVALID,
        GREYSCALE_8,
        RGB_8,
        RGBA_8
    };

    virtual ~Texture();

    void fromGreyscale(uchar* _data, uint _width, uint _height);
    void fromColour3(Colour3* _data, uint _width, uint _height);
    void fromColour4(Colour4* _data, uint _width, uint _height);

    void cleanUp();

    inline uint getWidth() const { return width; }
    inline uint getHeight() const { return height; }
    inline Mode getMode() const { return mode; }
    inline uchar* asGreyscale_8() const { return data.greyscale_8; }
    inline Colour3* asRGB_8() const { return data.rgb_8; }
    inline Colour4* asRGBA_8() const { return data.rgba_8; }
protected:
    virtual void loadFromFile(ifstream& file) override;
    virtual void saveToFile(ofstream& file) override;
private:
    Mode mode = INVALID;
    uint width;
    uint height;
    union {
        uchar* greyscale_8;
        Colour3* rgb_8;
        Colour4* rgba_8;
    } data;
};
