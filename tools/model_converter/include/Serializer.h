
#pragma once

#include <iostream>

#include "std.h"

#define SER_READ false
#define SER_WRITE true

class Serializer
{
public:
    Serializer(istream _src);
    Serializer(ostream _dst);

    void write_raw(char* buffer, int bytes);
    void read_raw(char* buffer, int bytes);

    void seek(uint offset);

    inline bool isWriting() const {
        return mode;
    }
    inline uint getSize() const {
        return size;
    }
private:
    bool mode;
    uint size;
    istream src;
    ostream dst;
};

template<typename T>
void write(Serializer& pkg, const T& data);

template<typename T>
void read(Serializer& pkg, T& data);

template<typename lengthType, typename T>
void write(Serializer& pkg, const T& data);

template<typename lengthType, typename T>
void read(Serializer& pkg, T& data);

template<typename lengthType, typename T>
void write_array(Serializer& pkg, T* data, lengthType len, bool omitLen = false);

template<typename lengthType, typename T>
void read_array(Serializer& pkg, T*& data, lengthType& len, bool omitLen = false);
