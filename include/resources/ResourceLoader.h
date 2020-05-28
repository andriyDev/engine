
#pragma once

#include "std.h"
#include "utility/Package.h"
#include "utility/Event.h"

#include <typeindex>

class Resource
{
public:
    /*
    Returns the list of dependencies for this resource.
    */
    virtual std::vector<uint> getDependencies() = 0;

    /*
    Call resolve on all of this resource's dependencies to collect their pointers.
    */
    virtual void resolveDependencies() = 0;

    /*
    Loads the resource. Returns true iff the resource is loaded successfully.
    data is the build data of the resource.
    */
    virtual bool load(std::shared_ptr<void> data) = 0;
};

enum class ResourceState : uchar
{
    Invalid, NotRequested, InProgress, Ready, Failed
};

template<typename T>
class ResourceRef
{
public:
    ResourceRef();
    ResourceRef(std::shared_ptr<T> _resource);
    ResourceRef(uint request);

    operator uint() {
        return id;
    }
    
    std::shared_ptr<T> resolve();

    inline ResourceState getState() const {
        return state;
    }
private:
    std::shared_ptr<T> resource;
    uint id;
    ResourceState state;
};

typedef std::shared_ptr<Resource> (*ResourceBuilder)(std::shared_ptr<void>);

class ResourceLoader
{
public:
    std::pair<std::shared_ptr<Resource>, ResourceState> resolve(uint resourceId);

    void loadStep();

    static ResourceLoader& get() {
        return loader;
    }
private:
    struct ResourceInfo
    {
    public:
        std::weak_ptr<Resource> ptr;
        std::shared_ptr<void> data;
        std::type_index type;
        ResourceState state;
    };

    std::shared_ptr<Resource> buildResource(uint resourceId);

    std::unordered_map<std::type_index, ResourceBuilder> builders;
    std::unordered_map<uint, ResourceInfo> resources;

    std::vector<std::pair<uint, std::shared_ptr<Resource>>> requests;
    std::vector<std::pair<uint, std::shared_ptr<Resource>>> loadStack;
    std::set<uint> loadSet;

    static ResourceLoader loader;
};
