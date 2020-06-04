
#include "resources/FileResource.h"

bool FileResource::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<FileData> fileData = dynamic_pointer_cast<FileData>(data);
    ifstream file(fileData->fileName, ios_base::binary | ios_base::in);
    if(!file.is_open()) {
        return false;
    }
    loadFromFile(file);
    file.close();
    return true;
}

bool FileResource::save(string fileName)
{
    ofstream file(fileName, ios_base::binary | ios_base::out);
    if(!file.is_open()) {
        return false;
    }
    saveToFile(file);
    file.close();
    return true;
}

shared_ptr<FileResource::FileData> FileResource::createAssetData(string fileName)
{
    shared_ptr<FileData> data = make_shared<FileData>();
    data->fileName = fileName;
    return data;
}
