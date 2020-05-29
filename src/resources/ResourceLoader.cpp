
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

std::pair<std::shared_ptr<Resource>, ResourceState> ResourceLoader::resolve(uint resourceId, ResolveMethod method)
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
    if(method == Immediate) {
        loadResource(resourceId);
    } else {
        requests.push_back(std::make_pair(resourceId, resource));
    }
    return std::make_pair(resource, res_pair->second.state);
}

std::shared_ptr<Resource> ResourceLoader::buildResource(uint resourceId)
{
    if(resourceId == 0) {
        return nullptr;
    }
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
    // Link the dependencies to the resource by resolving (use Deferred method since we are not loading here).
    resource->resolveDependencies(Deferred);
    return resource;
}

void ResourceLoader::loadStep()
{
    // Keep churning through requests until there are no more.
    while(!requests.empty()) {
        loadResource(requests.back().first);
        requests.pop_back();
    }
}

void ResourceLoader::loadResource(uint resourceId)
{
    // Implicitly assume that null resources are ready to go.
    if(resourceId == 0) {
        return;
    }

    auto res_pair = resources.find(resourceId);
    if(res_pair == resources.end()) {
        throw "Attempting to load non-existant resource.";
    }

    // Make sure this resource is built.
    std::shared_ptr<Resource> resource = buildResource(resourceId);
    for(uint dep_id : resource->getDependencies()) {
        loadResource(dep_id);
    }

    if(resource->load(res_pair->second.data)) {
        res_pair->second.state = ResourceState::Ready;
    } else {
        res_pair->second.state = ResourceState::Failed;
    }
}
