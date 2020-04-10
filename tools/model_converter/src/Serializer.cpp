
#include "Serializer.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

void Serializer::write_raw(char* buffer, int bytes)
{
    assert(mode == SER_WRITE);
    dst.write(buffer, bytes);
}

void Serializer::read_raw(char* buffer, int bytes)
{
    assert(mode == SER_READ);
    src.read(buffer, bytes);
}

void Serializer::seek(uint offset)
{

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

template<typename lengthType>
void write(Serializer& pkg, const string& data)
{
    lengthType lt = (lengthType)data.size();
    write(pkg, lt);
    pkg.write_raw(data.c_str(), lt);
}

template<typename lengthType>
void read(Serializer& pkg, string& data)
{
    lengthType lt;
    read(pkg, lt);
    char* c = malloc(lt + 1);
    c[lt] = 0;
    pkg.read_raw(c, lt);
    data = string(c);
    free(c);
}

template<typename lengthType, typename T>
void write_array(Serializer& pkg, T* data, lengthType len, bool omitLen)
{
    if(!omitLen) {
        write(pkg, len);
    }
    for(lengthType i = 0; i < len; i++) {
        write(pkg, data[i]);
    }
}

template<typename lengthType, typename T>
void read_array(Serializer& pkg, vector<T>& data, lengthType& len, bool omitLen)
{
    if(!omitLen) {
        lengthType l;
        read(pkg, l);
        len = l;
    }
    data.clear();
    for(lengthType i = 0; i < len; i++) {
        T element;
        read(pkg, element);
        data.push_back(element);
    }
}
