
#pragma once

#include "std.h"
#include "utility/Package.h"
#include "utility/Event.h"

class Resource : std::enable_shared_from_this<Resource>
{
public:
    enum State {
        NotInitialized,
        Queued,
        InProgress,
        Success,
        Failure
    };

    State state = NotInitialized;
    std::weak_ptr<class ResourceBuilder> builder;
    std::string resourceName;

    struct ResourceLoadDoneParams { std::shared_ptr<Resource> resource; };
    uint triggerOnLoad(ParamEvent<ResourceLoadDoneParams>::EventFcn fcn, void* data);
    void removeTriggerOnLoad(uint fcnId);

    inline uint getResourceType() const { return resourceTypeId; }

    inline bool isUsable() const { return state == Success; }
protected:
    Resource(uint _typeId) { resourceTypeId = _typeId; }
private:
    uint resourceTypeId;
    ParamEvent<ResourceLoadDoneParams> resourceLoadDone;

    friend class ResourceLoader;
};

class ResourceBuilder
{
public:
    float priority = 1.f; // The priority of this resource. This can only be changed before the ResourceLoader begins loading.

    inline Resource::State getState() const { return resource ? resource->state : Resource::NotInitialized; }
protected:
    /* Adds the dependency to the set of resources required to build this resource. */
    void addDependency(std::string dependencyName);

    template<typename T>
    std::shared_ptr<T> getDependency(std::string dependencyName, uint verifyType) const {
        auto it = dependencies.find(dependencyName);
        if(it == dependencies.end() || !it->second || it->second->state != Resource::Success || it->second->getResourceType() != verifyType) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(it->second);
    }
    
    template<typename T>
    std::shared_ptr<T> getResource() const {
        return std::static_pointer_cast<T>(resource);
    }

    /* Creates a resource pointer and returns it. */
    virtual std::shared_ptr<Resource> construct() = 0;

    /* Initializes the builder after the resource has been constructed. */
    virtual void init() {}

    /* Begins the process of modifying the resource into a ready state. Can also complete immediately. */
    virtual void startBuild() = 0;

    /* Forces the builder to complete immediately. Returns the state of the resource afterwards. */
    virtual Resource::State awaitBuild() { return resource ? resource->state : Resource::NotInitialized; }

    ResourceBuilder(uint constructedType);
private:
    /*
    The set of dependencies required to build the resource.
    Note these will be resolved after init is called, so ensure all dependencies
    are added at the end of init.
    */
    std::map<std::string, std::shared_ptr<Resource>> dependencies;
    std::shared_ptr<Resource> resource; // The pointer where we will store the resource once constructed.
    uint typeId; // The type id that this build will build.
    std::string resourceName; // The name of the resource we are building.

    friend class ResourceLoader;
};

class ResourceLoader
{
public:

    /*
    Adds the builder to the set of builders we want to build.
    If builder is null, this is treated as a "keep alive" message. If the resource was loaded or was being loaded,
    the resource will remain in the loader.
    */
    void addResource(const std::string& name, std::shared_ptr<ResourceBuilder> builder);

    /*
    Adds a resource directly to the set of loaded resources.
    The provided resource must have already been successfully loaded.
    */
    void addResource(const std::string& name, std::shared_ptr<Resource> resource);

    /* Removes the resource from the resource loader. */
    void releaseResource(const std::string& name);

    /*
    Removes resources not contained in the provided vector.
    Note we only consider resources that have been initialized to be able to be released.
    */
    void releaseUnusedResources(const std::set<std::string>& usedResources);

    /* Gets the list of resources that this loader is managing. */
    std::vector<std::string> getResourceNames() const;

    template<typename T>
    std::shared_ptr<T> getResource(const std::string& name, uint verificationTypeId) const {
        auto it = resources.find(name);
        if(it == resources.end() || !it->second) {
            return nullptr;
        }
        assert(it->second->getResourceType() == verificationTypeId);
        return std::static_pointer_cast<T>(it->second);
    }

    /*
    Initializes the loading process. Constructs resources, inits builders, and resolves dependency pointers.
    */
    void initLoad();

    /* Begins the loading process. */
    void beginLoad();

    /* Call regularly to continue load process. */
    void poll();
private:
    std::map<std::string, std::shared_ptr<ResourceBuilder>> pendingResources;
    std::map<std::string, std::shared_ptr<Resource>> resources;
    std::vector<std::shared_ptr<ResourceBuilder>> loadQueue;
    std::shared_ptr<ResourceBuilder> currentLoadTarget;
};
