
#pragma once

#include "utility/Serializer.h"

#include <functional>

#include "std.h"

struct Resource
{
    void* obj;
    uint typeId;
    string name;

    uint offset;
    uint length;

    Resource() {}
};

typedef function<void(Serializer&, void*)> WriteFcn;
typedef function<void*(Serializer&)> ReadFcn;

class Package
{
public:
    Package(Serializer _serializer, map<uint, pair<WriteFcn, ReadFcn>>* _parsers);

    // Loads the basic package info. Does not load resources immediately.
    void loadPackage();
    // Either gets the specified resource, or loads it from the serializer.
    void* getResource(string name);
    // Lists all available resources (by name) of the specified type.
    vector<string> getResourcesByType(uint typeId) const;
    // Lists all available resources with their name and type.
    vector<pair<string, uint>> getAllResources() const;
    // Adds the resource to the package to later be saved out.
    void addResource(string name, uint typeId, void* obj);
    // Writes the resources to the serializer (does not necessarily save it to disk).
    void savePackage();
    // Frees up all loaded resources.
    void freeResources();

    inline bool isWriting() const {
        return serializer.isWriting();
    }
private:
    Serializer serializer; // The serializer to read/write from.
    map<uint, pair<WriteFcn, ReadFcn>>* parsers; // Functions to read and write the specified type.
    map<string, Resource> resources; // The resources available.
};
