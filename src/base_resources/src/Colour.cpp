
#include "resources/Colour.h"

void read_Colour3_inplace(istream* buf, Colour3* colour)
{
    colour->r = read_uchar(buf);
    colour->g = read_uchar(buf);
    colour->b = read_uchar(buf);
}

void read_Colour4_inplace(istream* buf, Colour4* colour)
{
    colour->r = read_uchar(buf);
    colour->g = read_uchar(buf);
    colour->b = read_uchar(buf);
    colour->a = read_uchar(buf);
}

void write_Colour3(ostream* buf, const Colour3& colour)
{
    write_uchar(buf, colour.r);
    write_uchar(buf, colour.g);
    write_uchar(buf, colour.b);
}

void write_Colour4(ostream* buf, const Colour4& colour)
{
    write_uchar(buf, colour.r);
    write_uchar(buf, colour.g);
    write_uchar(buf, colour.b);
    write_uchar(buf, colour.a);
}
