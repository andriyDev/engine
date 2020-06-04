
#pragma once

#include <typeindex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace std;
using namespace glm;

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

template<typename K, typename V>
using hash_map = unordered_map<K, V>;
template<typename T>
using hash_set = unordered_set<T>;

#define get_id(x) (uint)(typeid(x).hash_code())
