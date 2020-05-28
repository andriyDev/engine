
#include "resources/ResourceLoader.h"

#include <algorithm>

using ::ResourceState;

template<typename T>
ResourceRef<T>::ResourceRef()
    : resource(nullptr), id(0), state(Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(std::shared_ptr<T> _resource)
    : resource(_resource), id(0), state(_resource ? Ready : Invalid)
{ }

template<typename T>
ResourceRef<T>::ResourceRef(uint request)
    : id(request), resource(nullptr), state(NotRequested)
{ }

template<typename T>
std::shared_ptr<T> ResourceRef<T>::resolve()
{
    // These are all the cases in which we should just used the cached (or null-ed) value.
    if(state != ResourceState::NotRequested && state != ResourceState::InProgress || id == 0) {
        return resource;
    }

    // This means we need to collect the resource from the loader.
    auto response = ResourceLoader::get().resolve(id);
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

std::pair<std::shared_ptr<Resource>, ResourceState> ResourceLoader::resolve(uint resourceId)
{
    auto res_pair = resources.find(resourceId);
    if(res_pair == resources.end() || res_pair->second.state == ResourceState::Failed) {
        // Cannot find resource.
        return std::make_pair(nullptr, ResourceState::Invalid);
    }
    
    std::shared_ptr<Resource> resource = res_pair->second.ptr.lock();
    if(resource) {
        return std::make_pair(resource, res_pair->second.state);
    }

    // We need to build the resource.
    auto builder_pair = builders.find(res_pair->second.type);
    if(builder_pair == builders.end()) {
        return std::make_pair(nullptr, ResourceState::Failed);
    }

    resource = builder_pair->second(res_pair->second.data);
    res_pair->second.ptr = resource;
    requests.push_back(resourceId);
    return std::make_pair(resource, ResourceState::InProgress);
}

void ResourceLoader::loadStep()
{
    // If there is nothing being loaded, start loading something.
    if(loadStack.size() == 0) {
        // If there are no requests, we're done.
        if(requests.size() == 0) {
            // Only clear the load cache once there are no more requests.
            loadCache.clear();
            return;
        }

        loadStack.push_back(requests[0]);
        requests.erase(requests.begin());
    }

    while(true) {
        uint id = loadStack.back();
        
        auto res_pair = resources.find(id);
        if(res_pair == resources.end()) {
            throw "Resource info for resource in load queue not found.";
        }
        std::shared_ptr<Resource> resource = res_pair->second.ptr.lock();
        if(!resource) {
            // We need to build the resource.
            auto builder_pair = builders.find(res_pair->second.type);
            if(builder_pair == builders.end()) {
                throw "Unable to build resource dependency.";
            }

            resource = builder_pair->second(res_pair->second.data);
            res_pair->second.ptr = resource;
            loadCache.push_back(resource);
            res_pair->second.state = ResourceState::InProgress;
        }

        bool dependenciesReady = true;
        for(uint dep_id : resource->getDependencies()) {
            auto dep_pair = resources.find(dep_id);
            if(dep_pair == resources.end()) {
                throw "Resource dependency doesn't exist!";
            }
            if(dep_pair->second.ptr.expired()) {
                dependenciesReady = false;
                dep_pair->second.state = ResourceState::Invalid;
                loadStack.push_back(dep_id);
            }
        }

        if(dependenciesReady) {
            if(resource->load(res_pair->second.data)) {
                res_pair->second.state = ResourceState::Ready;
            }
            else {
                res_pair->second.state = ResourceState::Failed;
            }
            loadStack.erase(loadStack.end() - 1);
        }
    }
}
