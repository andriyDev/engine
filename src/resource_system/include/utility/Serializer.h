
#pragma once

#include <iostream>

#include "std.h"

float read_float(istream* buf);
int read_int(istream* buf);
short read_short(istream* buf);
char read_char(istream* buf);

uint read_uint(istream* buf);
ushort read_ushort(istream* buf);
uchar read_uchar(istream* buf);

string read_string(istream* buf, uint str_len);
string read_string_uint_len(istream* buf);
string read_string_ushort_len(istream* buf);
string read_string_uchar_len(istream* buf);

void write_float(ostream* buf, float v);
void write_int(ostream* buf, int v);
void write_short(ostream* buf, short v);
void write_char(ostream* buf, char v);

void write_uint(ostream* buf, uint v);
void write_ushort(ostream* buf, ushort v);
void write_uchar(ostream* buf, uchar v);

void write_string(ostream* buf, const string& str, uint n);
void write_string_uint_len(ostream* buf, const string& str);
void write_string_ushort_len(ostream* buf, const string& str);
void write_string_uchar_len(ostream* buf, const string& str);



vec2 read_vec2(istream* buf);
vec3 read_vec3(istream* buf);
vec4 read_vec4(istream* buf);

void read_vec2_inplace(istream* buf, vec2* v);
void read_vec3_inplace(istream* buf, vec3* v);
void read_vec4_inplace(istream* buf, vec4* v);

void write_vec2(ostream* buf, const vec2& v);
void write_vec3(ostream* buf, const vec3& v);
void write_vec4(ostream* buf, const vec4& v);

vec2 compress_unit(const vec3& v);
vec3 decompress_unit(const vec2& v);
