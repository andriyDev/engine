
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

    resource = buildResource(resourceId);
    requests.push_back(std::make_pair(resourceId, resource));
    return std::make_pair(resource, ResourceState::InProgress);
}

std::shared_ptr<Resource> ResourceLoader::buildResource(uint resourceId)
{
    // First find the resource. We assume this passes.
    auto res_pair = resources.find(resourceId);
    // If the resource is already built, we assume its children are also built.
    std::shared_ptr<Resource> resource = res_pair->second.ptr.lock();
    if(resource) {
        return resource;
    }
    
    // Next find its builder.
    auto builder_pair = builders.find(res_pair->second.type);
    if(builder_pair == builders.end()) {
        return nullptr;
    }

    // Build the resource and add it to the resource info map.
    resource = builder_pair->second(res_pair->second.data);
    res_pair->second.ptr = resource;
    res_pair->second.state = ResourceState::InProgress;

    // Build all dependencies for this resource as well.
    std::vector<std::shared_ptr<Resource>> dependencies;
    for(uint dependency : resource->getDependencies()) {
        dependencies.push_back(buildResource(dependency));
    }
    // Link the dependencies to the resource by resolving.
    resource->resolveDependencies();
    return resource;
}

void ResourceLoader::loadStep()
{
    // Keep churning through requests until there are no more.
    while(true) {
        // If there is nothing being loaded, start loading something.
        if(loadStack.size() == 0) {
            // If there are no requests, we're done.
            if(requests.size() == 0) {
                return;
            }

            loadStack.push_back(requests[0]);
            loadSet.insert(requests[0].first);
            requests.erase(requests.begin());
        }

        // We now get the id and resource from the loadStack.
        uint id = loadStack.back().first;
        std::shared_ptr<Resource> resource = loadStack.back().second;
        
        // Find the resource info.
        auto res_pair = resources.find(id);
        if(res_pair == resources.end()) {
            throw "Resource info for resource in load queue not found.";
        }
        // We now need to ensure the resource's dependencies are all complete.
        bool dependenciesReady = true;
        for(uint dep_id : resource->getDependencies()) {
            auto dep_pair = resources.find(dep_id);
            if(dep_pair == resources.end()) {
                throw "Resource dependency doesn't exist!";
            }
            if(dep_pair->second.state != ResourceState::Ready) {
                // This code is ugly but...
                // Construct the pair corresponding to the dependency (for the load queue).
                std::pair<uint, std::shared_ptr<Resource>> dep_pair2
                    = std::make_pair(dep_id, dep_pair->second.ptr.lock());
                // Check if the loadSet contains the dependency.
                // If it does, we want to find the element in the load queue and move it to the end.
                // Otherwise, we just want to push the new item to the load queue.
                // In either case, loadSet contains the dependency Id afterwards.
                if(loadSet.find(dep_id) == loadSet.end()) {
                    loadSet.insert(dep_id);
                } else {
                    loadStack.erase(std::find(loadStack.begin(), loadStack.end(), dep_pair2));
                }
                loadStack.push_back(dep_pair2);
                dependenciesReady = false;
            }
        }

        // If the dependencies are all loaded, we need to load this resource.
        if(dependenciesReady) {
            if(resource->load(res_pair->second.data)) {
                res_pair->second.state = ResourceState::Ready;
            }
            else {
                res_pair->second.state = ResourceState::Failed;
            }
            // Tell the load set it no longer contains this id.
            loadSet.erase(id);
            // Remove this element from the load stack (so we move to the previous item).
            loadStack.erase(loadStack.end() - 1);
        }
    }
}
