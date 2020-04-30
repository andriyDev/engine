
#pragma once

#include "utility/Serializer.h"

#include <functional>
#include <fstream>

#include "std.h"

struct Resource
{
    void* obj;
    uint typeId;
    std::string name;

    uint offset;
    uint length;

    Resource() {}
};

typedef std::function<void(Serializer&, void*)> WriteFcn;
typedef std::function<void*(Serializer&)> ReadFcn;

class Package
{
public:
    Package();
    Package(Serializer _serializer, const uchar* _typeCode, std::map<uint, std::pair<WriteFcn, ReadFcn>>* _parsers);

    // Loads the basic package info. Does not load resources immediately.
    void loadPackage();
    // Either gets the specified resource, or loads it from the serializer.
    void* getResource(std::string name);
    template<typename T>
    T* getResource(std::string name) {
        return static_cast<T*>(getResource(name));
    }
    // Lists all available resources (by name) of the specified type.
    std::vector<std::string> getResourcesByType(uint typeId) const;
    // Lists all available resources with their name and type.
    std::vector<std::pair<std::string, uint>> getAllResources() const;
    // Adds the resource to the package to later be saved out.
    void addResource(std::string name, uint typeId, void* obj);
    // Writes the resources to the serializer (does not necessarily save it to disk).
    void savePackage();
    // Frees up all loaded resources.
    void freeResources();
    /*
    Gets the specified resource, but does not save it to the package.
    This resource then won't be deleted on freeResources.
    If the resource is already cached, will return and release that.
    */
    void* releaseResource(std::string name);
    template<typename T>
    T* releaseResource(std::string name) {
        return static_cast<T*>(releaseResource(name));
    }

    inline bool isWriting() const {
        return serializer.isWriting();
    }
private:
    Serializer serializer; // The serializer to read/write from.
    std::map<uint, std::pair<WriteFcn, ReadFcn>>* parsers; // Functions to read and write the specified type.
    std::map<std::string, Resource> resources; // The resources available.

    uchar typeCode[3]; // The type code to expect from the stored data.

    friend class PackageFile;
};

class PackageFile
{
public:
    PackageFile(std::string _fileName, const uchar* _typeCode, std::map<uint, std::pair<WriteFcn, ReadFcn>>* _parsers);

    void open();

    void* releaseResource(std::string name);
    template<typename T>
    T* releaseResource(std::string name) {
        return static_cast<T*>(releaseResource(name));
    }

    void close();

    inline bool isOpen() const {
        return bIsOpen;
    }
private:
    std::string fileName;
    std::ifstream file;
    Package pack;
    std::map<std::string, Resource> resources;
    uchar typeCode[3];
    bool init = false;
    bool bIsOpen = false;

    std::map<uint, std::pair<WriteFcn, ReadFcn>>* parsers;
};
