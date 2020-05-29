
#pragma once

#include "std.h"
#include "FileResource.h"

class Shader : public FileResource
{
public:
    std::string code;
protected:
    virtual void loadFromFile(std::ifstream& file) override;
    virtual void saveToFile(std::ofstream& file) override;
};
