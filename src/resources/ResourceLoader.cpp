
#include "resources/ResourceLoader.h"

#include <algorithm>

uint Resource::triggerOnLoad(ParamEvent<ResourceLoadDoneParams>::EventFcn fcn, void* data)
{
    if(state == Success || state == Failure) {
        fcn(data, {shared_from_this()});
        return 0;
    } else {
        return resourceLoadDone.addFunction(fcn, data);
    }
}

void Resource::removeTriggerOnLoad(uint fcnId)
{
    resourceLoadDone.removeFunction(fcnId);
}

ResourceBuilder::ResourceBuilder(uint constructedType) {
    typeId = constructedType;
}

void ResourceBuilder::addDependency(std::string dependencyName)
{
    dependencies.insert(std::make_pair(dependencyName, std::shared_ptr<Resource>()));
}

void ResourceLoader::addResource(const std::string& name, std::shared_ptr<ResourceBuilder> builder)
{
    assert(builder);
    builder->resourceName = name;
    // TODO: Handle clobbering better.
    pendingResources.insert_or_assign(name, builder);
}

void ResourceLoader::addResource(const std::string& name, std::shared_ptr<Resource> resource)
{
    assert(resource && resource->isUsable());
    resources.insert_or_assign(name, resource);
}

void ResourceLoader::releaseResource(const std::string& name)
{
    pendingResources.erase(name);
    if(currentLoadTarget && currentLoadTarget->resourceName == name) {
        currentLoadTarget = nullptr;
    }

    auto mit = resources.find(name);
    if(mit == resources.end()) {
        return;
    }
    if(mit->second->state == Resource::Queued) {
        for(auto it = loadQueue.begin(); it != loadQueue.end(); ++it) {
            if((*it)->resourceName == name) {
                loadQueue.erase(it);
                break;
            }
        }
    }
}

void ResourceLoader::releaseUnusedResources(const std::set<std::string>& usedResources)
{
    std::vector<std::string> unusedResources;
    for(auto pair : resources) {
        if(usedResources.find(pair.first) == usedResources.end()) {
            unusedResources.push_back(pair.first);
        }
    }
    for(std::string res : unusedResources) {
        releaseResource(res);
    }
}

std::vector<std::string> ResourceLoader::getResourceNames() const
{
    std::vector<std::string> names;
    for(auto pair : resources) {
        names.push_back(pair.first);
    }
    return names;
}

void ResourceLoader::initLoad()
{
    // Construct and init each resource.
    for(auto res_pair : pendingResources) {
        if(!res_pair.second->resource) {
            res_pair.second->resource = res_pair.second->construct();
            res_pair.second->resource->resourceName = res_pair.first;
            resources.insert(std::make_pair(res_pair.first, res_pair.second->resource));
            res_pair.second->resource->builder = res_pair.second;
            res_pair.second->init();
        }
    }

    // Go through each resource.
    for(auto res_pair : pendingResources) {
        std::map<std::string, std::shared_ptr<Resource>>& resourceDependencies = res_pair.second->dependencies;
        // Go through each dependency of the resource.
        for(auto& dep_pair : resourceDependencies) {
            // No reason to try to resolve a resource
            if(dep_pair.second) {
                continue;
            }
            // Look up the resource in the set of resources we hold (possibly unbuilt).
            auto dep = resources.find(dep_pair.first);
            // If we don't find the resource, the dependency is invalid.
            if(dep == resources.end()) {
                throw "Dependency not found!";
            }
            // Set the dependency pointer to be the resource the builder is building.
            dep_pair.second = dep->second;
            // If the pointer was set to null, the dependency hasn't been constructed, so abort.
            if(!dep_pair.second) {
                throw "Dependency not constructed!";
            }
        }
    }
}

void ResourceLoader::beginLoad()
{
    std::vector<std::shared_ptr<ResourceBuilder>> allResources;
    std::set<ResourceBuilder*> children;
    // Go through each pending resource and add it to the list of resources we will load.
    for(auto resource : pendingResources) {
        allResources.push_back(resource.second);
    }
    // Add the previous load queue to the set so we make sure to reorganize it.
    allResources.insert(allResources.end(), loadQueue.begin(), loadQueue.end());

    // Go through the resource's dependencies and mark them that they are children.
    for(const std::shared_ptr<ResourceBuilder>& resource : allResources) {
        resource->resource->state = Resource::Queued;
        for(auto dep_pair : resource->dependencies) {
            std::shared_ptr<ResourceBuilder> ptr;
            if(ptr = dep_pair.second->builder.lock()) {
                children.insert(ptr.get());
            }
        }
    }

    // Go through each "root" resource and have it update the priority of all its descendents.
    for(std::shared_ptr<ResourceBuilder>& resource : allResources) {
        // Skip resources that are not children.
        if(children.find(resource.get()) != children.end()) {
            continue;
        }

        // This structure is nonsense
        std::vector< // Stack of
            std::pair< // Pairs of 
                ResourceBuilder*, // ResourceBuilders and 
                std::map< // Iterators for the dependencies of ResourceBuilders.
                    std::string, std::shared_ptr<Resource>
                >::iterator
            >
        > stack;
        // We now push our initial state onto the stack. The "root" resource, starting at its first child.
        stack.push_back(std::make_pair(resource.get(), resource->dependencies.begin()));
        while(!stack.empty()) {
            // Get the most recent entry.
            auto& last = stack.back();
            // Once the iterator is referring to the last of the parent's dependencies,
            // we can discard this child and move to our parent's next child.
            if(last.second == last.first->dependencies.end()) {
                stack.pop_back();
                continue;
            }
            // If the resource is already usable, we don't need to queue its builder.
            if(last.second->second->isUsable()) {
                last.second++;
            }
            else {
                // Get the next child.
                ResourceBuilder* child = last.second->second->builder.lock().get();
                // Move to the next child (so when we come back we move on).
                last.second++;
                // Update its priority,
                child->priority += last.first->priority + 1;
                // Add the child to the stack, pointing to the first child.
                stack.push_back(std::make_pair(child, child->dependencies.begin()));
            }
        }
    }

    // Sort the resources with increasing priority. Later we will pop from the back,
    // so it's more like a reversed decreasing priority array.
    std::sort(allResources.begin(), allResources.end(),
        [](const std::shared_ptr<ResourceBuilder>& A, const std::shared_ptr<ResourceBuilder>& B) {
            return A->priority < B->priority;
    });
    loadQueue = allResources;
    pendingResources.clear();
}

void ResourceLoader::poll() {
    if(currentLoadTarget) {
        if(currentLoadTarget->resource->state == Resource::Success
            || currentLoadTarget->resource->state == Resource::Failure) {
            currentLoadTarget->resource->resourceLoadDone.dispatch({currentLoadTarget->resource});
            currentLoadTarget->resource->resourceLoadDone.clearFunctions();
            currentLoadTarget = nullptr;
        }
    }

    if(!currentLoadTarget && !loadQueue.empty()) {
        currentLoadTarget = loadQueue.back();
        loadQueue.pop_back();
        currentLoadTarget->resource->state = Resource::InProgress;
        currentLoadTarget->startBuild();
    }
}
