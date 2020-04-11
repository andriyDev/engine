
#include "utility/Package.h"

Package::Package(Serializer _serializer, map<uint, pair<WriteFcn, ReadFcn>>* _parsers)
    : serializer(_serializer), parsers(_parsers)
{
    assert(_parsers);
}

struct Header
{
    uchar code[3]; // Must be PKG
    string versionCode;
};

template<>
void write(Serializer& pkg, const Header& data) {
    assert(data.code[0] == 'P' && data.code[1] == 'K' && data.code[2] == 'G');
    write_array<uchar>(pkg, data.code, 3, true);
    write_string<uchar>(pkg, data.versionCode);
}
template<>
void read(Serializer& pkg, Header& data) {
    vector<char> code; uchar len = 3;
    read_array<uchar, char>(pkg, code, len, true);
    if(code[0] != 'P' || code[1] != 'K' || code[2] != 'G') {
        throw 1; // Not a pkg file!
    }
    read_string<uchar>(pkg, data.versionCode);
}

struct FileInfo
{
    uint offset;
    uint length;
    uchar type;
    string name;
};

template<>
void write(Serializer& pkg, const FileInfo& data) {
    write(pkg, data.offset);
    write(pkg, data.length);
    write(pkg, data.type);
    write_string<uchar>(pkg, data.name);
}
template<>
void read(Serializer& pkg, FileInfo& data) {
    read(pkg, data.offset);
    read(pkg, data.length);
    read(pkg, data.type);
    read_string<uchar>(pkg, data.name);
}

void Package::loadPackage()
{
    assert(!isWriting());

    Header header;
    read(serializer, header);
    if(header.versionCode != "1.0") {
        fprintf(stderr, "Bad version\n");
        throw 1;
    }

    vector<FileInfo> files;
    read_array<uchar>(serializer, files);
    for(FileInfo& file : files) {
        Resource res;
        res.obj = nullptr;
        res.name = file.name;
        res.typeId = file.type;
        res.offset = file.offset;
        res.length = file.length;
        if(parsers->find(file.type) == parsers->end()) {
            fprintf(stderr, "Invalid resource type id (%d)\n", file.type);
            throw 1;
        }
        resources.insert(make_pair(res.name, res));
    }
}

void* loadResource(Serializer& ser, Resource& res, map<uint, pair<WriteFcn, ReadFcn>>& parsers)
{
    ser.seek(res.offset, SER_START);

    auto it = parsers.find(res.typeId);
    res.obj = it->second.second(ser);

    return res.obj;
}

void* Package::getResource(string name)
{
    auto it = resources.find(name);
    Resource res;
    memcpy(&res, &it->second, sizeof(Resource));
    if(res.obj) {
        return res.obj;
    }
    res.obj = loadResource(serializer, res, *parsers);
    resources.insert(it, make_pair(res.name, res));
    return res.obj;
}

vector<string> Package::getResourcesByType(uint typeId) const
{
    assert(typeId != 0);
    vector<string> resourceNames;
    for(auto p : resources) {
        if(p.second.typeId == typeId) {
            resourceNames.push_back(p.first);
        }
    }
    return resourceNames;
}

vector<pair<string, uint>> Package::getAllResources() const
{
    vector<pair<string, uint>> resourceData;
    for(auto p : resources) {
        resourceData.push_back(make_pair(p.first, p.second.typeId));
    }
    return resourceData;
}

void Package::addResource(string name, uint typeId, void* obj)
{
    Resource res;
    res.name = name;
    res.obj = obj;
    res.length = 0;
    res.offset = 0;
    res.typeId = typeId;
    if(parsers->find(typeId) == parsers->end()) {
        fprintf(stderr, "Invalid resource type id (%d)\n", typeId);
        throw 1;
    }
    resources.insert(make_pair(name, res));
}

void saveResource(Serializer& ser, Resource& res, map<uint, pair<WriteFcn, ReadFcn>>& parsers)
{
    res.offset = ser.pos();

    auto it = parsers.find(res.typeId);
    it->second.first(ser, res.obj);

    res.length = ser.pos() - res.offset;
}

void Package::savePackage()
{
    assert(isWriting());

    Header header;
    header.code[0] = 'P'; header.code[1] = 'K'; header.code[2] = 'G';
    header.versionCode = "1.0";
    write(serializer, header);

    vector<FileInfo> files;
    vector<Resource> resList;
    files.reserve(resources.size());
    resList.reserve(resources.size());
    for(auto p : resources) {
        FileInfo f;
        f.name = p.first;
        f.type = p.second.typeId;
        f.offset = serializer.pos();
        f.length = 0;
        files.push_back(f);
        resList.push_back(p.second);
        write(serializer, f);
    }
    
    for(Resource& res : resList) {
        saveResource(serializer, res, *parsers);
    }

    for(int i = 0; i < resList.size(); i++) {
        FileInfo& file = files[i];
        Resource& res = resList[i];

        serializer.seek(file.offset, SER_START);
        write(serializer, res.offset);
        write(serializer, res.length);
    }
}

void Package::freeResources()
{
    for(auto p : resources) {
        if(p.second.obj) {
            delete p.second.obj;
        }
    }
}
