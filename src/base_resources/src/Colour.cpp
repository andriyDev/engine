
#include "resources/Colour.h"

template<>
void write(Serializer& ser, const Colour3& colour)
{
    write(ser, colour.r);
    write(ser, colour.g);
    write(ser, colour.b);
}

template<>
void read(Serializer& ser, Colour3& colour)
{
    read(ser, colour.r);
    read(ser, colour.g);
    read(ser, colour.b);
}

template<>
void write(Serializer& ser, const Colour4& colour)
{
    write(ser, colour.r);
    write(ser, colour.g);
    write(ser, colour.b);
    write(ser, colour.a);
}

template<>
void read(Serializer& ser, Colour4& colour)
{
    read(ser, colour.r);
    read(ser, colour.g);
    read(ser, colour.b);
    read(ser, colour.a);
}
