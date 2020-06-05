
#pragma once

#include "std.h"

#include "resources/ResourceLoader.h"

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

class FontLoader : public Resource
{
public:
    virtual ~FontLoader();
    static void registerLoader(uint id);
protected:
    FontLoader() {}

    virtual vector<uint> getDependencies() override { return vector<uint>(); }
    virtual void resolveDependencies(ResolveMethod method) override { }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        return shared_ptr<FontLoader>(new FontLoader());
    }
private:
    FT_Library freeType = nullptr;

    friend class FontFace;
};

class FontFace : public Resource
{
public:
    FontFace(ResourceRef<FontLoader> _loader, string fileName, uint fontFaceIndex = 0);
    virtual ~FontFace();

    static shared_ptr<BuildData> createAssetData(uint loaderId, const string& fileName, uint fontFaceIndex = 0);

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data);
protected:
    class BuildData : public Resource::BuildData
    {
    public:
        uint loaderId;
        string fileName;
        uint faceIndex;
    };

    FontFace() {}

    virtual vector<uint> getDependencies() override { return { loader }; }
    virtual void resolveDependencies(ResolveMethod method) override { loader.resolve(method); }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

    ResourceRef<FontLoader> loader;
private:
    FT_Face face_data = nullptr;

    friend class Font;
};
