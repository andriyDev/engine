
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"
#include <fstream>

class FileResource : public Resource
{
public:
    bool save(std::string fileName);

    template<typename T>
    static std::shared_ptr<Resource> build(std::shared_ptr<Resource::BuildData> data) {
        return std::shared_ptr<T>(new T());
    }

    class FileData : public Resource::BuildData
    {
    public:
        std::string fileName;
    };

    static std::shared_ptr<FileData> createAssetData(std::string fileName);
protected:
    virtual std::vector<uint> getDependencies() override { return {}; }
    virtual void resolveDependencies(ResolveMethod method) override {}
    virtual bool load(std::shared_ptr<Resource::BuildData> data) override;
    
    virtual void loadFromFile(std::ifstream& file) = 0;
    virtual void saveToFile(std::ofstream& file) = 0;
};
