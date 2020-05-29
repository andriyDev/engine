
#include "resources/ResourceLoader.h"

#include <algorithm>

using ::ResourceState;

ResourceLoader ResourceLoader::loader;

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

bool ResourceLoader::loadResource(uint resourceId)
{
    // Implicitly assume that null resources are ready to go.
    if(resourceId == 0) {
        return true;
    }

    auto res_pair = resources.find(resourceId);
    if(res_pair == resources.end()) {
        throw "Attempting to load non-existant resource.";
    }

    // Make sure this resource is built.
    std::shared_ptr<Resource> resource = buildResource(resourceId);
    for(uint dep_id : resource->getDependencies()) {
        if(!loadResource(dep_id)) {
            res_pair->second.state = ResourceState::Failed;
            fprintf(stderr, "Failed to load resource %d due to dependency %d.\n", resourceId, dep_id);
            return false;
        }
    }

    if(resource->load(res_pair->second.data)) {
        printf("Loaded %d\n", resourceId);
        res_pair->second.state = ResourceState::Ready;
        return true;
    } else {
        res_pair->second.state = ResourceState::Failed;
        fprintf(stderr, "Failed to load resource %d.\n", resourceId);
        return false;
    }
}

void ResourceLoader::addResource(uint resourceId, std::shared_ptr<Resource> resource)
{
    ResourceInfo info;
    info.ptr = resource;
    info.data = nullptr;
    info.type = typeid(ResourceInfo);
    info.state = ResourceState::Ready;
    resources.insert_or_assign(resourceId, info);
}

void ResourceLoader::removeResource(uint resourceId)
{
    resources.erase(resourceId);
}

void ResourceLoader::addAssetType(std::type_index type, ResourceBuilder builder)
{
    builders.insert_or_assign(type, builder);
}

void ResourceLoader::addAssetData(uint resourceId, std::type_index type, std::shared_ptr<Resource::BuildData> buildData)
{
    ResourceInfo info;
    info.ptr = std::shared_ptr<Resource>(nullptr);
    info.data = buildData;
    info.type = type;
    info.state = ResourceState::NotRequested;
    resources.insert_or_assign(resourceId, info);
}

