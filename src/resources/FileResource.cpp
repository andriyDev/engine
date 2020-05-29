
#include "resources/FileResource.h"

bool FileResource::load(std::shared_ptr<Resource::BuildData> data)
{
    std::shared_ptr<FileData> fileData = std::dynamic_pointer_cast<FileData>(data);
    std::ifstream file(fileData->fileName, std::ios_base::binary | std::ios_base::in);
    if(!file.is_open()) {
        return false;
    }
    loadFromFile(file);
    file.close();
    return true;
}

bool FileResource::save(std::string fileName)
{
    std::ofstream file(fileName, std::ios_base::binary | std::ios_base::out);
    if(!file.is_open()) {
        return false;
    }
    saveToFile(file);
    file.close();
    return true;
}

std::shared_ptr<FileResource::FileData> FileResource::createAssetData(std::string fileName)
{
    std::shared_ptr<FileData> data = std::make_shared<FileData>();
    data->fileName = fileName;
    return data;
}
