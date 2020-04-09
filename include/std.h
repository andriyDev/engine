
#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>

using namespace std;

#ifdef NDEBUG
#define assert(x)
#define check(x) (x)
#else
#define assert(x) if(!(x)) { fprintf(stderr, "Assertion failed! Line %d, File: %s\n", __LINE__, __FILE__); throw 1; }
#define check(x) (x) ? (x) : throw "Invalid pointer"
#endif

typedef unsigned int uint;
