
#pragma once

#include "std.h"

#include "utility/Serializer.h"

struct Colour3
{
    union {
        uchar comp[3];
        struct{
            uchar r;
            uchar g;
            uchar b;
        };
    };
};

struct Colour4
{
    union {
        uchar comp[4];
        struct{
            uchar r;
            uchar g;
            uchar b;
            uchar a;
        };
    };
};

void read_Colour3_inplace(istream* buf, Colour3* colour);
void read_Colour4_inplace(istream* buf, Colour4* colour);

void write_Colour3(ostream* buf, const Colour3& colour);
void write_Colour4(ostream* buf, const Colour4& colour);
