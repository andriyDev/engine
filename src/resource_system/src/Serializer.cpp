
#include "utility/Serializer.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <math.h>

float read_float(istream* buf)
{
    union {
        uint i;
        float f;
    };
    i = read_uint(buf);
    return f;
}

int read_int(istream* buf)
{
    int data;
    buf->read((char*)&data, sizeof(int));
    return ntohl(data);
}

short read_short(istream* buf)
{
    short data;
    buf->read((char*)&data, sizeof(short));
    return ntohs(data);
}

char read_char(istream* buf)
{
    char data;
    buf->read((char*)&data, sizeof(char));
    return data;
}

uint read_uint(istream* buf)
{
    uint data;
    buf->read((char*)&data, sizeof(uint));
    return ntohl(data);
}

ushort read_ushort(istream* buf)
{
    ushort data;
    buf->read((char*)&data, sizeof(ushort));
    return ntohs(data);
}

uchar read_uchar(istream* buf)
{
    uchar data;
    buf->read((char*)&data, sizeof(uchar));
    return data;
}

string read_string(istream* buf, uint str_len)
{
    char* char_buf = new char[str_len + 1];
    char_buf[str_len] = 0;
    buf->read(char_buf, str_len);
    string result(char_buf);
    delete char_buf;
    return result;
}

string read_string_uint_len(istream* buf)
{
    uint l = read_uint(buf);
    return read_string(buf, l);
}

string read_string_ushort_len(istream* buf)
{
    ushort l = read_ushort(buf);
    return read_string(buf, (uint)l);
}

string read_string_uchar_len(istream* buf)
{
    uchar l = read_uchar(buf);
    return read_string(buf, (uint)l);
}

void write_float(ostream* buf, float v)
{
    union {
        uint i;
        float f;
    };
    f = v;
    write_uint(buf, i);
}

void write_int(ostream* buf, int v)
{
    uint v2 = htonl(v);
    buf->write((char*)&v2, sizeof(uint));
}

void write_short(ostream* buf, short v)
{
    ushort v2 = htons(v);
    buf->write((char*)&v2, sizeof(ushort));
}

void write_char(ostream* buf, char v)
{
    buf->write((char*)v, sizeof(char));
}

void write_uint(ostream* buf, uint v)
{
    v = htonl(v);
    buf->write((char*)&v, sizeof(uint));
}

void write_ushort(ostream* buf, ushort v)
{
    v = htons(v);
    buf->write((char*)&v, sizeof(ushort));
}

void write_uchar(ostream* buf, uchar v)
{
    buf->write((char*)&v, sizeof(uchar));
}

void write_string(ostream* buf, const string& str, uint n)
{
    buf->write(str.c_str(), min(str.size(), n));
}

void write_string_uint_len(ostream* buf, const string& str)
{
    uint n = (uint)str.size();
    write_uint(buf, n);
    write_string(buf, str, n);
}

void write_string_ushort_len(ostream* buf, const string& str)
{
    ushort n = (ushort)str.size();
    write_ushort(buf, n);
    write_string(buf, str, n);
}

void write_string_uchar_len(ostream* buf, const string& str)
{
    uchar n = (uchar)str.size();
    write_ushort(buf, n);
    write_string(buf, str, n);
}

vec2 read_vec2(istream* buf)
{
    vec2 v;
    v.x = read_float(buf);
    v.y = read_float(buf);
    return v;
}

vec3 read_vec3(istream* buf)
{
    vec3 v;
    v.x = read_float(buf);
    v.y = read_float(buf);
    v.z = read_float(buf);
    return v;
}

vec4 read_vec4(istream* buf)
{
    vec4 v;
    v.x = read_float(buf);
    v.y = read_float(buf);
    v.z = read_float(buf);
    v.w = read_float(buf);
    return v;
}

void read_vec2_inplace(istream* buf, vec2* v)
{
    v->x = read_float(buf);
    v->y = read_float(buf);
}

void read_vec3_inplace(istream* buf, vec3* v)
{
    v->x = read_float(buf);
    v->y = read_float(buf);
    v->z = read_float(buf);
}

void read_vec4_inplace(istream* buf, vec4* v)
{
    v->x = read_float(buf);
    v->y = read_float(buf);
    v->z = read_float(buf);
    v->w = read_float(buf);
}

void write_vec2(ostream* buf, const vec2& v)
{
    write_float(buf, v.x);
    write_float(buf, v.y);
}

void write_vec3(ostream* buf, const vec3& v)
{
    write_float(buf, v.x);
    write_float(buf, v.y);
    write_float(buf, v.z);
}

void write_vec4(ostream* buf, const vec4& v)
{
    write_float(buf, v.x);
    write_float(buf, v.y);
    write_float(buf, v.z);
    write_float(buf, v.w);
}

vec2 compress_unit(const vec3& v)
{
    return vec2(v.x, v.y);
}

vec3 decompress_unit(const vec2& v)
{
    return vec3(v.x, v.y, sqrtf(1.0f - v.x*v.x - v.y * v.y));
}
