
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
    virtual std::vector<uint> getDependencies() = 0;

    /*
    Call resolve on all of this resource's dependencies to collect their pointers.
    */
    virtual void resolveDependencies(ResolveMethod method) = 0;

    /*
    Loads the resource. Returns true iff the resource is loaded successfully.
    data is the build data of the resource.
    */
    virtual bool load(std::shared_ptr<BuildData> data) = 0;

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
    ResourceRef(std::shared_ptr<T> _resource);
    ResourceRef(uint request);

    operator uint() {
        return id;
    }
    
    std::shared_ptr<T> resolve(ResolveMethod method);

    inline ResourceState getState() const {
        return state;
    }
private:
    std::shared_ptr<T> resource;
    uint id;
    ResourceState state;
};

typedef std::shared_ptr<Resource> (*ResourceBuilder)(std::shared_ptr<Resource::BuildData>);

class ResourceLoader
{
public:
    std::pair<std::shared_ptr<Resource>, ResourceState> resolve(uint resourceId, ResolveMethod method);

    void loadStep();
    bool loadResource(uint resourceId);

    void addResource(uint resourceId, std::shared_ptr<Resource> resource);
    void removeResource(uint resourceId);
    void addAssetType(std::type_index type, ResourceBuilder builder);
    void addAssetData(uint resourceId, std::type_index type, std::shared_ptr<Resource::BuildData> buildData);

    static ResourceLoader& get() {
        return loader;
    }
private:
    struct ResourceInfo
    {
        std::weak_ptr<Resource> ptr;
        std::shared_ptr<Resource::BuildData> data;
        std::type_index type = std::type_index(typeid(ResourceInfo));
        ResourceState state;
    };

    std::shared_ptr<Resource> buildResource(uint resourceId);

    std::unordered_map<std::type_index, ResourceBuilder> builders;
    std::unordered_map<uint, ResourceInfo> resources;

    std::vector<std::pair<uint, std::shared_ptr<Resource>>> requests;

    // Private constructor.
    ResourceLoader() {}

    static ResourceLoader loader;
};

template<typename T>
ResourceRef<T>::ResourceRef()
    : resource(nullptr), id(0), state(ResourceState::Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(std::shared_ptr<T> _resource)
    : resource(_resource), id(0), state(_resource ? ResourceState::Ready : ResourceState::Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(uint request)
    : id(request), resource(nullptr), state(ResourceState::NotRequested)
{ }

template<typename T>
std::shared_ptr<T> ResourceRef<T>::resolve(ResolveMethod method)
{
    // These are all the cases in which we should just used the cached (or null-ed) value.
    if(state != ResourceState::NotRequested && state != ResourceState::InProgress || id == 0) {
        return resource;
    }

    // This means we need to collect the resource from the loader.
    auto response = ResourceLoader::get().resolve(id, method);
    resource = std::dynamic_pointer_cast<T>(response.first);
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