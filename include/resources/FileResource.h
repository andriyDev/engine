
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"

class FileResource : public Resource
{
public:
    virtual std::vector<uint> getDependencies() override { return {}; }

    virtual bool load(std::shared_ptr<void> data) override;
    bool save(std::string fileName);

    template<typename T>
    static std::shared_ptr<Resource> build(std::shared_ptr<void> data) {
        return std::make_shared<T>();
    }
protected:
    virtual void loadFromFile(std::ifstream& file) = 0;
    virtual void saveToFile(std::ofstream& file) = 0;
private:
    struct FileData
    {
        std::string fileName;
    };
};
