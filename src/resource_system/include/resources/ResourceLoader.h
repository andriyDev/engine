
#pragma once

#include "std.h"

#include <typeindex>

enum ResolveMethod : uchar
{
    Immediate,
    Deferred
};

class Resource
{
public:
    class BuildData
    {
    public:
        virtual ~BuildData() {}
    };
protected:
    /*
    Returns the list of dependencies for this resource.
    */
    virtual vector<uint> getDependencies() = 0;

    /*
    Call resolve on all of this resource's dependencies to collect their pointers.
    */
    virtual void resolveDependencies(ResolveMethod method) = 0;

    /*
    Loads the resource. Returns true iff the resource is loaded successfully.
    data is the build data of the resource.
    */
    virtual bool load(shared_ptr<BuildData> data) = 0;

    friend class ResourceLoader;
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
    ResourceRef(shared_ptr<T> _resource);
    ResourceRef(uint request);

    operator uint() {
        return id;
    }
    
    shared_ptr<T> resolve(ResolveMethod method);

    inline ResourceState getState() const {
        return state;
    }
private:
    shared_ptr<T> resource;
    uint id;
    ResourceState state;
};

typedef shared_ptr<Resource> (*ResourceBuilder)(shared_ptr<Resource::BuildData>);

class ResourceLoader
{
public:
    pair<shared_ptr<Resource>, ResourceState> resolve(uint resourceId, ResolveMethod method);

    void loadStep();
    bool loadResource(uint resourceId);

    void addResource(uint resourceId, shared_ptr<Resource> resource);
    void removeResource(uint resourceId);
    void addAssetType(type_index type, ResourceBuilder builder);
    void addAssetData(uint resourceId, type_index type, shared_ptr<Resource::BuildData> buildData);

    static ResourceLoader& get() {
        return loader;
    }
private:
    struct ResourceInfo
    {
        weak_ptr<Resource> ptr;
        shared_ptr<Resource::BuildData> data;
        type_index type = type_index(typeid(ResourceInfo));
        ResourceState state;
    };

    shared_ptr<Resource> buildResource(uint resourceId);

    hash_map<type_index, ResourceBuilder> builders;
    hash_map<uint, ResourceInfo> resources;

    vector<pair<uint, shared_ptr<Resource>>> requests;

    // Private constructor.
    ResourceLoader() {}

    static ResourceLoader loader;
};

template<typename T>
ResourceRef<T>::ResourceRef()
    : resource(nullptr), id(0), state(ResourceState::Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(shared_ptr<T> _resource)
    : resource(_resource), id(0), state(_resource ? ResourceState::Ready : ResourceState::Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(uint request)
    : id(request), resource(nullptr), state(ResourceState::NotRequested)
{ }

template<typename T>
shared_ptr<T> ResourceRef<T>::resolve(ResolveMethod method)
{
    // These are all the cases in which we should just used the cached (or null-ed) value.
    if(state != ResourceState::NotRequested && state != ResourceState::InProgress || id == 0) {
        return resource;
    }

    // This means we need to collect the resource from the loader.
    auto response = ResourceLoader::get().resolve(id, method);
    resource = dynamic_pointer_cast<T>(response.first);
    state = response.second;
    // If the loader succeeded in acquiring the resource, but the cast failed,
    // mark it as a failure.
    if(state == ResourceState::Ready && !resource) {
        state = ResourceState::Failed;
    }
    if(state == ResourceState::Failed || state == ResourceState::Invalid) {
        resource = nullptr;
    }
    return state == ResourceState::Ready ? resource : nullptr;
}