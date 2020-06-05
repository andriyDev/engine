
#include "font/FontFace.h"

#include <ft2build.h>
#include <freetype/freetype.h>

FontLoader::~FontLoader()
{
    if(freeType) {
        FT_Done_FreeType(freeType);
    }
}

void FontLoader::registerLoader(uint id)
{
    ResourceLoader& res = ResourceLoader::get();
    res.addAssetType(typeid(FontLoader), FontLoader::build);
    res.addAssetData(id, typeid(FontLoader), nullptr);
}

bool FontLoader::load(shared_ptr<Resource::BuildData> data)
{
    if(FT_Init_FreeType(&freeType)) {
        fprintf(stderr, "Failed to load FreeType\n");
        return false;
    }
    return true;
}

FontFace::FontFace(ResourceRef<FontLoader> _loader, string fileName, uint fontFaceIndex)
    : loader(_loader)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->fileName = fileName;
    data->faceIndex = fontFaceIndex;
    resolveDependencies(Immediate);
    if(!load(data)) {
        throw "Failed to load FontFace";
    }
}

shared_ptr<Resource::BuildData> FontFace::createAssetData(uint loaderId, const string& fileName, uint fontFaceIndex)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->loaderId = loaderId;
    data->fileName = fileName;
    data->faceIndex = fontFaceIndex;
    return data;
}

shared_ptr<Resource> FontFace::build(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<BuildData> d = static_pointer_cast<BuildData>(data);
    shared_ptr<FontFace> face(new FontFace());
    face->loader = d->loaderId;
    return face;
}

FontFace::~FontFace()
{
    if(face_data) {
        FT_Done_Face(face_data);
    }
}

bool FontFace::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<BuildData> d = static_pointer_cast<BuildData>(data);
    shared_ptr<FontLoader> loaderPtr = loader.resolve(Immediate);
    if(FT_New_Face(loaderPtr->freeType, d->fileName.c_str(), d->faceIndex, &face_data)) {
        fprintf(stderr, "Failed to load FontFace from '%s'\n", d->fileName.c_str());
        return false;
    }
    return true;
}
