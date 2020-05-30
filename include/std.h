
#pragma once

#include <typeindex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <memory>

#ifdef NDEBUG
#define assert(x)
#define check(x) (x)
#else
#define assert(x) if(!(x)) { fprintf(stderr, "Assertion failed! Line %d, File: %s\n", __LINE__, __FILE__); throw 1; }
#define check(x) ((x) ? (x) : throw "Invalid pointer")
#endif

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define get_id(x) (uint)(typeid(x).hash_code())
