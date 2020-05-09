
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

template<>
void write(Serializer& ser, const Colour3& colour);

template<>
void read(Serializer& ser, Colour3& colour);

template<>
void write(Serializer& ser, const Colour4& colour);

template<>
void read(Serializer& ser, Colour4& colour);
