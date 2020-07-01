
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"
#include <fstream>

class FileResource : public Resource
{
public:
    bool save(string fileName);

    template<typename T>
    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        return shared_ptr<T>(new T());
    }

    template<typename T>
    static shared_ptr<T> loadDirectly(string fileName) {
        shared_ptr<T> resource = make_shared<T>();
        return resource->load(createAssetData(fileName)) ? resource : nullptr;
    }

    class FileData : public Resource::BuildData
    {
    public:
        string fileName;
    };

    static shared_ptr<FileData> createAssetData(string fileName);
protected:
    virtual vector<uint> getDependencies() override { return {}; }
    virtual void resolveDependencies(ResolveMethod method) override {}
    virtual bool load(shared_ptr<Resource::BuildData> data) override;
    
    virtual void loadFromFile(ifstream& file) = 0;
    virtual void saveToFile(ofstream& file) = 0;
};
