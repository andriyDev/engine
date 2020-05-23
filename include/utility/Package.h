
#pragma once

#include "utility/Serializer.h"

#include <functional>
#include <fstream>

#include "std.h"

typedef std::function<void(Serializer&, void*)> WriteFcn;
typedef std::function<void*(Serializer&)> ReadFcn;
typedef std::function<void(Serializer&, void*)> ReadIntoFcn;

class Package
{
public:
    struct Resource
    {
        void* obj;
        uint typeId;
        std::string name;

        uint offset;
        uint length;

        Resource() {}
    };

    Package();
    Package(Serializer _serializer, const uchar* _typeCode, std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>>* _parsers);

    // Loads the basic package info. Does not load resources immediately.
    void loadPackage();
    // Either gets the specified resource, or loads it from the serializer. Also returns the type id of the resource.
    std::pair<void*, uint> getResource(std::string name);
    template<typename T>
    T* getResource(std::string name, uint typeId) {
        std::pair<void*, uint> resource = getResource(name);
        assert(resource.second == typeId);
        return static_cast<T*>(resources.first);
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
    std::pair<void*, uint> releaseResource(std::string name);
    template<typename T>
    T* releaseResource(std::string name, uint typeId) {
        std::pair<void*, uint> resource = releaseResource(name);
        assert(resource.second == typeId);
        return static_cast<T*>(resource.first);
    }

    void loadIntoResource(std::string name, void* resource, uint typeId);

    inline bool isWriting() const {
        return serializer.isWriting();
    }
private:
    Serializer serializer; // The serializer to read/write from.
    std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>>* parsers; // Functions to read and write the specified type.
    std::map<std::string, Package::Resource> resources; // The resources available.

    uchar typeCode[3]; // The type code to expect from the stored data.

    friend class PackageFile;
};

class PackageFile
{
public:
    PackageFile(std::string _fileName, const uchar* _typeCode, std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>>* _parsers);
    virtual ~PackageFile();

    void open();

    void loadIntoResource(std::string name, void* resource, uint typeId);

    std::pair<void*, uint> releaseResource(std::string name);
    template<typename T>
    T* releaseResource(std::string name, uint typeId) {
        std::pair<void*, uint> resource = releaseResource(name);
        assert(resource.second == typeId);
        return static_cast<T*>(resource.first);
    }

    bool hasResource(std::string name) {
        return init && resources.find(name) != resources.end();
    }

    void close();

    inline bool isOpen() const {
        return bIsOpen;
    }
private:
    std::string fileName;
    std::ifstream file;
    Package pack;
    std::map<std::string, Package::Resource> resources;
    uchar typeCode[3];
    bool init = false;
    bool bIsOpen = false;

    std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>>* parsers;
};
