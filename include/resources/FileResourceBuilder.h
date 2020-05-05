
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"

template<typename T>
class FileResourceBuilder : public ResourceBuilder
{
public:
    FileResourceBuilder(uint constructedType, std::shared_ptr<PackageFile> _package, std::string _nameInFile, uint _fileResourceType)
        : ResourceBuilder(constructedType), package(_package), nameInFile(_nameInFile),
            fileResourceType(_fileResourceType) {
        assert(package);
    }
protected:

    virtual std::shared_ptr<Resource> construct() override {
        return std::make_shared<T>();
    }
    
    virtual void startBuild() override {
        if(!package->isOpen()) {
            package->open();
        }
        package->loadIntoResource(nameInFile, getResource<T>().get(), fileResourceType);
        getResource<T>()->state = Resource::Success;
    }

    std::shared_ptr<PackageFile> package;
    uint fileResourceType;
    std::string nameInFile;
};
