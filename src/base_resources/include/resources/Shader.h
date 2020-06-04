
#pragma once

#include "std.h"
#include "resources/FileResource.h"

class Shader : public FileResource
{
public:
    string code;
protected:
    virtual void loadFromFile(ifstream& file) override;
    virtual void saveToFile(ofstream& file) override;
};
