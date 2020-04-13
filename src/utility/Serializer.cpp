
#include "utility/Serializer.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

Serializer::Serializer()
    : src(nullptr), dst(nullptr)
{ }

Serializer::Serializer(istream* _src)
    : src(_src), dst(nullptr)
{ }

Serializer::Serializer(ostream* _dst)
    : dst(_dst), src(nullptr)
{ }

void Serializer::write_raw(char* buffer, int bytes)
{
    assert(dst);
    dst->write(buffer, bytes);
}

void Serializer::read_raw(char* buffer, int bytes)
{
    assert(src);
    src->read(buffer, bytes);
}

void Serializer::seek(uint offset, uint relativeTo)
{
    assert(relativeTo < 3);
    ios_base::seekdir base;
    if(relativeTo == SER_START) {
        base = ios_base::beg;
    } else if(relativeTo == SER_END) {
        base = ios_base::end;
    } else {
        base = ios_base::cur;
    }

    if(dst) {
        dst->seekp(offset, base);
    } else {
        src->seekg(offset, base);
    }
}

uint Serializer::pos()
{
    if(dst) {
        return (uint)dst->tellp();
    } else {
        return (uint)src->tellg();
    }
}

template<>
void write(Serializer& pkg, const int& data)
{
    int netData = htonl(data);
    pkg.write_raw((char*)&netData, sizeof(int));
}

template<>
void read(Serializer& pkg, int& data)
{
    int netData;
    pkg.read_raw((char*)&netData, sizeof(int));
    data = ntohl(netData);
}

template<>
void write(Serializer& pkg, const float& data)
{
    union {
        float netDataF;
        int netDataL;
    };
    netDataF = data;
    netDataL = htonl(netDataL);
    pkg.write_raw((char*)&netDataL, sizeof(float));
}

template<>
void read(Serializer& pkg, float& data)
{
    union {
        float netDataF;
        int netDataL;
    };
    pkg.read_raw((char*)&netDataL, sizeof(float));
    netDataL = ntohl(netDataL);
    data = netDataF;
}

template<>
void write(Serializer& pkg, const short& data)
{
    short netData = htons(data);
    pkg.write_raw((char*)&netData, sizeof(short));
}

template<>
void read(Serializer& pkg, short& data)
{
    short netData;
    pkg.read_raw((char*)&netData, sizeof(short));
    data = ntohs(netData);
}

template<>
void write(Serializer& pkg, const char& data)
{
    pkg.write_raw((char*)&data, sizeof(char));
}

template<>
void read(Serializer& pkg, char& data)
{
    pkg.read_raw(&data, sizeof(char));
}

// ===== UNSIGNED ===== //

template<>
void write(Serializer& pkg, const uint& data)
{
    uint netData = htonl(data);
    pkg.write_raw((char*)&netData, sizeof(uint));
}

template<>
void read(Serializer& pkg, uint& data)
{
    uint netData;
    pkg.read_raw((char*)&netData, sizeof(uint));
    data = ntohl(netData);
}

template<>
void write(Serializer& pkg, const ushort& data)
{
    ushort netData = htons(data);
    pkg.write_raw((char*)&netData, sizeof(ushort));
}

template<>
void read(Serializer& pkg, ushort& data)
{
    ushort netData;
    pkg.read_raw((char*)&netData, sizeof(ushort));
    data = ntohs(netData);
}

template<>
void write(Serializer& pkg, const uchar& data)
{
    pkg.write_raw((char*)&data, sizeof(uchar));
}

template<>
void read(Serializer& pkg, uchar& data)
{
    pkg.read_raw((char*)&data, sizeof(uchar));
}
