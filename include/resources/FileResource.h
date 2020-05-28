
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"

class FileResource : public Resource
{
public:
    bool save(std::string fileName);

    template<typename T>
    static std::shared_ptr<Resource> build(std::shared_ptr<void> data) {
        return std::make_shared<T>();
    }
protected:
    virtual std::vector<uint> getDependencies() override { return {}; }
    virtual void resolveDependencies() override {}
    virtual bool load(std::shared_ptr<void> data) override;
    
    virtual void loadFromFile(std::ifstream& file) = 0;
    virtual void saveToFile(std::ofstream& file) = 0;
private:
    struct FileData
    {
        std::string fileName;
    };
};
