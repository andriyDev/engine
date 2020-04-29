
#include "utility/Package.h"

Package::Package()
    : serializer(Serializer()), parsers(nullptr),
    resources()
{ }

Package::Package(Serializer _serializer, const uchar* _typeCode, map<uint, pair<WriteFcn, ReadFcn>>* _parsers)
    : serializer(_serializer), parsers(_parsers), resources()
{
    assert(_parsers);
    typeCode[0] = _typeCode[0];
    typeCode[1] = _typeCode[1];
    typeCode[2] = _typeCode[2];
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
    data.code[0] = code[0]; data.code[1] = code[1]; data.code[2] = code[2];
    if(code[0] != 'P' || code[1] != 'K' || code[2] != 'G') {
        throw 1; // Not a pkg file!
    }
    read_string<uchar>(pkg, data.versionCode);
}

struct V1_0_Header
{
    uchar typeCode[3];
};

template<>
void write(Serializer& pkg, const V1_0_Header& data) {
    write_array<uchar>(pkg, data.typeCode, 3, true);
}
template<>
void read(Serializer& pkg, V1_0_Header& data) {
    vector<char> code; uchar len = 3;
    read_array<uchar, char>(pkg, code, len, true);
    data.typeCode[0] = code[0]; data.typeCode[1] = code[1]; data.typeCode[2] = code[2];
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
    V1_0_Header vheader;
    read(serializer, vheader);

    assert(vheader.typeCode[0] == typeCode[0]
        && vheader.typeCode[1] == typeCode[1]
        && vheader.typeCode[2] == typeCode[2]);

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
    assert(it != resources.end());
    Resource& res = it->second;
    if(res.obj) {
        return res.obj;
    }
    res.obj = loadResource(serializer, res, *parsers);
    return res.obj;
}

void* Package::releaseResource(string name)
{
    auto it = resources.find(name);
    assert(it != resources.end());
    Resource& res = it->second;
    if(res.obj) {
        void* result = res.obj;
        res.obj = nullptr;
        return result;
    }
    return loadResource(serializer, res, *parsers);
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

    V1_0_Header vheader;
    vheader.typeCode[0] = typeCode[0]; vheader.typeCode[1] = typeCode[1]; vheader.typeCode[2] = typeCode[2];
    write(serializer, vheader);

    write<uchar>(serializer, (uchar)resources.size());
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

PackageFile::PackageFile(string _fileName, const uchar* _typeCode, map<uint, pair<WriteFcn, ReadFcn>>* _parsers)
    : fileName(_fileName), parsers(_parsers)
{
    typeCode[0] = _typeCode[0];
    typeCode[1] = _typeCode[1];
    typeCode[2] = _typeCode[2];
}

void PackageFile::open()
{
    assert(!bIsOpen);
    file.open(fileName, ios::binary);
    if(!file.is_open()) {
        fprintf(stderr, "File not found: %s\n", fileName.c_str());
        throw 1;
    }
    pack = Package(Serializer(&file), typeCode, parsers);
    bIsOpen = true;

    if(!init) {
        pack.loadPackage();
        resources = pack.resources;
    } else {
        pack.resources = resources;
    }
    init = true;
}

void* PackageFile::releaseResource(string name)
{
    assert(bIsOpen);
    return pack.releaseResource(name);
}

void PackageFile::close()
{
    assert(bIsOpen);
    file.close();
    pack.freeResources(); // Just in case.
    pack.resources.clear();
    pack = Package();
    bIsOpen = false;
}
