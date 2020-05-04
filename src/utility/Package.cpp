
#include "utility/Package.h"

Package::Package()
    : serializer(Serializer()), parsers(nullptr),
    resources()
{ }

Package::Package(Serializer _serializer, const uchar* _typeCode, std::map<uint, std::pair<WriteFcn, ReadFcn>>* _parsers)
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
    std::string versionCode;
};

template<>
void write(Serializer& pkg, const Header& data) {
    assert(data.code[0] == 'P' && data.code[1] == 'K' && data.code[2] == 'G');
    write_array<uchar>(pkg, data.code, 3, true);
    write_string<uchar>(pkg, data.versionCode);
}
template<>
void read(Serializer& pkg, Header& data) {
    std::vector<char> code; uchar len = 3;
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
    std::vector<char> code; uchar len = 3;
    read_array<uchar, char>(pkg, code, len, true);
    data.typeCode[0] = code[0]; data.typeCode[1] = code[1]; data.typeCode[2] = code[2];
}

struct FileInfo
{
    uint offset;
    uint length;
    uchar type;
    std::string name;
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

    std::vector<FileInfo> files;
    read_array<uchar>(serializer, files);
    for(FileInfo& file : files) {
        Package::Resource res;
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

void* loadResource(Serializer& ser, Package::Resource& res, std::map<uint, std::pair<WriteFcn, ReadFcn>>& parsers)
{
    ser.seek(res.offset, SER_START);

    auto it = parsers.find(res.typeId);
    res.obj = it->second.second(ser);

    return res.obj;
}

std::pair<void*, uint> Package::getResource(std::string name)
{
    auto it = resources.find(name);
    assert(it != resources.end());
    Package::Resource& res = it->second;
    if(res.obj) {
        return std::make_pair(res.obj, res.typeId);
    }
    res.obj = loadResource(serializer, res, *parsers);
    return std::make_pair(res.obj, res.typeId);
}

std::pair<void*, uint> Package::releaseResource(std::string name)
{
    auto it = resources.find(name);
    assert(it != resources.end());
    Package::Resource& res = it->second;
    if(res.obj) {
        void* result = res.obj;
        res.obj = nullptr;
        return std::make_pair(result, res.typeId);
    }
    return std::make_pair(loadResource(serializer, res, *parsers), res.typeId);
}

std::vector<std::string> Package::getResourcesByType(uint typeId) const
{
    assert(typeId != 0);
    std::vector<std::string> resourceNames;
    for(auto p : resources) {
        if(p.second.typeId == typeId) {
            resourceNames.push_back(p.first);
        }
    }
    return resourceNames;
}

std::vector<std::pair<std::string, uint>> Package::getAllResources() const
{
    std::vector<std::pair<std::string, uint>> resourceData;
    for(auto p : resources) {
        resourceData.push_back(make_pair(p.first, p.second.typeId));
    }
    return resourceData;
}

void Package::addResource(std::string name, uint typeId, void* obj)
{
    Package::Resource res;
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

void saveResource(Serializer& ser, Package::Resource& res, std::map<uint, std::pair<WriteFcn, ReadFcn>>& parsers)
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
    std::vector<FileInfo> files;
    std::vector<Resource> resList;
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

PackageFile::PackageFile(std::string _fileName, const uchar* _typeCode,
    std::map<uint, std::pair<WriteFcn, ReadFcn>>* _parsers)
    : fileName(_fileName), parsers(_parsers)
{
    typeCode[0] = _typeCode[0];
    typeCode[1] = _typeCode[1];
    typeCode[2] = _typeCode[2];
}

PackageFile::~PackageFile()
{
    if(bIsOpen) {
        close();
    }
}

void PackageFile::open()
{
    assert(!bIsOpen);
    file.open(fileName, std::ios::binary);
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

std::pair<void*, uint> PackageFile::releaseResource(std::string name)
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
