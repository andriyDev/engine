
#pragma once

#include <iostream>

#include "std.h"

#define SER_READ false
#define SER_WRITE true

#define SER_START 0
#define SER_END 1
#define SER_CUR 2

class Serializer
{
public:
    Serializer(istream* _src);
    Serializer(ostream* _dst);

    void write_raw(char* buffer, int bytes);
    void read_raw(char* buffer, int bytes);

    void seek(uint offset, uint relativeTo);
    uint pos();

    inline bool isWriting() const {
        return dst;
    }
private:
    istream* src;
    ostream* dst;
};

template<typename T>
void write(Serializer& pkg, const T& data);

template<typename T>
void read(Serializer& pkg, T& data);

template<typename lengthType, typename T>
void write_array(Serializer& pkg, T const* data, lengthType len, bool omitLen = false) 
{
    if(!omitLen) {
        write(pkg, len);
    }
    for(lengthType i = 0; i < len; i++) {
        write(pkg, data[i]);
    }
}

template<typename lengthType, typename T>
void read_array(Serializer& pkg, vector<T>& data, lengthType len=0, bool omitLen = false)
{
    if(!omitLen) {
        read(pkg, len);
    }
    data.clear();
    for(lengthType i = 0; i < len; i++) {
        T element;
        read(pkg, element);
        data.push_back(element);
    }
}

template<typename lengthType, typename T>
void read_array(Serializer& pkg, T* data, lengthType len=0, bool omitLen = false)
{
    if(!omitLen) {
        read(pkg, len);
    }
    for(lengthType i = 0; i < len; i++) {
        T element;
        read(pkg, element);
        data[i] = element;
    }
}

template<typename lengthType, typename T>
void read_array_alloc(Serializer& pkg, T*& data, lengthType& len, bool omitLen = false)
{
    if(!omitLen) {
        read(pkg, len);
    }
    data = (T*)new uchar[len * sizeof(T)];
    for(lengthType i = 0; i < len; i++) {
        T element;
        read(pkg, element);
        data[i] = element;
    }
}

template<typename lengthType>
void write_string(Serializer& pkg, const string& data, bool omitLen=false)
{
    write_array<lengthType>(pkg, data.c_str(), (lengthType)data.size(), omitLen);
}

template<typename lengthType>
void read_string(Serializer& pkg, string& data, int len=0, bool omitLen=false)
{
    vector<char> raw_data;
    raw_data.push_back(0);
    read_array<lengthType>(pkg, raw_data, len, omitLen);
    data = string(raw_data.begin(), raw_data.end());
}
