
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "std.h"
#include "resources/Mesh.h"
#include "resources/Shader.h"
#include "resources/Texture.h"
#include "font/Font.h"
#include <ft2build.h>
#include <freetype/freetype.h>

#include "utility/Serializer.h"

#include <png.h>
#include <stdio.h>

vector<string> splitByDelimiter(string str, char delim)
{
    vector<string> result;
    stringstream ss(str);
    string token;
    while(getline(ss, token, delim)) {
        result.push_back(token);
    }
    return move(result);
}

string field(string src, char delim, int entry)
{
    vector<string> split = splitByDelimiter(src, delim);
    return entry < 0 ? split[split.size() + entry] : split[entry];
}

void findMeshes(aiNode* node, const aiScene* scene, vector<string>& meshNames, vector<aiMesh*>& meshes)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        string meshName = mesh->mName.C_Str();

        if(meshNames.size() == 0) {
            meshes.push_back(mesh);
            return;
        } else {
            auto it = find(meshNames.begin(), meshNames.end(), meshName);
            if(it != meshNames.end()) {
                meshes.push_back(mesh);
                meshNames.erase(it);
                if(meshNames.size() == 0) {
                    return;
                }
            }
        }
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        findMeshes(node->mChildren[i], scene, meshNames, meshes);
        if(meshNames.size() == 0 && meshes.size() > 0) {
            return;
        }
    }
}

#define USAGE "Usage: model_converter <out_file> <type_code>\n"

Mesh* convertMesh(aiMesh* srcMesh)
{
    Mesh* mesh = new Mesh();
    mesh->vertCount = srcMesh->mNumVertices;
    mesh->vertData = (Mesh::Vertex*)malloc(mesh->vertCount * sizeof(Mesh::Vertex));

    aiVector3D* p = srcMesh->mVertices;
    aiVector3D* n = srcMesh->mNormals;
    aiVector3D* t = srcMesh->mTangents;
    aiVector3D* b = srcMesh->mBitangents;
    aiVector3D* u = srcMesh->mTextureCoords[0];
    aiColor4D* c = srcMesh->mColors[0];

    for(uint i = 0; i < mesh->vertCount; i++) {
        mesh->vertData[i].position  = vec3(p[i].x, p[i].y, p[i].z);
        mesh->vertData[i].normal    = n ? vec3(n[i].x, n[i].y, n[i].z) : vec3(1,0,0);
        mesh->vertData[i].tangent   = t ? vec3(t[i].x, t[i].y, t[i].z) : vec3(0,1,0);
        mesh->vertData[i].bitangent = b ? vec3(b[i].x, b[i].y, b[i].z) : vec3(0,0,1);
        mesh->vertData[i].texCoord  = u ? vec2(u[i].x, u[i].y) : vec2(0, 0);
        mesh->vertData[i].colour    = c ? vec4(c[i].r, c[i].g, c[i].b, c[i].a) : vec4(1, 1, 1, 1);
    }
    mesh->indexCount = srcMesh->mNumFaces * 3;
    mesh->indexData = new uint[mesh->indexCount];

    for(uint i = 0; i < srcMesh->mNumFaces; i++) {
        mesh->indexData[i * 3    ] = srcMesh->mFaces[i].mIndices[0];
        mesh->indexData[i * 3 + 1] = srcMesh->mFaces[i].mIndices[1];
        mesh->indexData[i * 3 + 2] = srcMesh->mFaces[i].mIndices[2];
    }

    // TODO: Extract the bone data if it exists.
    return mesh;
}

vector<pair<Mesh*, string>> extractMeshes(const string& fileName,
    vector<string> desiredMeshes, hash_map<string, string> meshNameMap, Assimp::Importer& importer)
{
    vector<pair<Mesh*, string>> resources;

    const aiScene* scene = importer.ReadFile(fileName,
        aiProcess_Triangulate
        | aiProcess_GenSmoothNormals
        | aiProcess_CalcTangentSpace
        | aiProcess_GenUVCoords);
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "Conversion error: " << importer.GetErrorString() << endl;
        return resources;
    }

    vector<string> modifiedDesiredMeshes = desiredMeshes;
    vector<aiMesh*> resultingMeshes;
    findMeshes(scene->mRootNode, scene, modifiedDesiredMeshes, resultingMeshes);
    for(aiMesh* mesh : resultingMeshes) {
        auto it = meshNameMap.find(mesh->mName.C_Str());
        assert(it != meshNameMap.end());
        resources.push_back(make_pair(convertMesh(mesh), it->second));
    }
    importer.FreeScene();
    return resources;
}

Texture* loadPNGTexture(const string& fileName)
{
    FILE* f;
    if(fopen_s(&f, fileName.c_str(), "rb")) {
        fprintf(stderr, "Failed to read file '%s'.\n", fileName.c_str());
        return nullptr;
    }
    
    uchar header[8];
    fread(header, 1, 8, f);
    if(png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Failed to read file as png '%s'.\n", fileName.c_str());
        fclose(f);
        return nullptr;
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        fprintf(stderr, "Failed to create png read struct.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return nullptr;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        fprintf(stderr, "Failed to create png info struct.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return nullptr;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "init_io error (png stuff).\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return nullptr;
    }

    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int colourType = png_get_color_type(png_ptr, info_ptr);
    int bitDepth = png_get_bit_depth(png_ptr, info_ptr);
    
    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

    Texture* tex = nullptr;
    if(bitDepth == 8 && colourType == PNG_COLOR_TYPE_RGB) {
        Colour3* data = new Colour3[width * height];
        for(int y = 0; y < height; y++) {
            png_bytep row = row_pointers[y];
            for(int x = 0; x < width; x++) {
                data[x + y * width] = { row[x * 3], row[x * 3 + 1], row[x * 3 + 2] };
            }
        }
        tex = new Texture();
        tex->fromColour3(data, width, height);
    } else if(bitDepth == 8 && colourType == PNG_COLOR_TYPE_RGBA) {
        Colour4* data = new Colour4[width * height];
        for(int y = 0; y < height; y++) {
            png_bytep row = row_pointers[y];
            for(int x = 0; x < width; x++) {
                data[x + y * width] = { row[x * 4], row[x * 4 + 1], row[x * 4 + 2], row[x * 4 + 3] };
            }
        }
        tex = new Texture();
        tex->fromColour4(data, width, height);
    } else {
        fprintf(stderr, "Unrecognized colour type or bit depth in png: %d/%d.\n", colourType, bitDepth);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return tex;
}

#define FONT_CHAR_ROW 10

pair<Font*, Texture*> loadFont(string fontFile, uint faceIndex, uint size)
{
    FT_Library freeType;
    FT_Face face;

    if(FT_Init_FreeType(&freeType)) {
        cerr << "Failed to load FreeType" << endl;
        return make_pair(nullptr, nullptr);
    }

    if(FT_New_Face(freeType, fontFile.c_str(), faceIndex, &face)) {
        cerr << "Failed to load FontFace from '" << fontFile << "'" << endl;
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }

    Font* font = new Font();

    FT_Set_Pixel_Sizes(face, 0, size);

    uvec2 currentOffset(0,0);
    uvec2 texDimensions(0,0);

    int orderIndex = 0;

    font->lineHeight = (ushort)(face->size->metrics.height >> 6);
    font->sourceFontSize = (ushort)size;

    if(FT_Load_Char(face, 32, FT_LOAD_ADVANCE_ONLY)) {
        fprintf(stderr, "Could not load space info during Font loading.\n");

        delete font;
        FT_Done_Face(face);
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }
    font->spaceAdvance = (uchar)(face->glyph->advance.x >> 6);

    font->maxDescent = 0;

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_BITMAP_METRICS_ONLY | FT_LOAD_ADVANCE_ONLY)) {
            font->characters[c].valid = false;
        } else {
            font->characters[c].valid = true;
            // The position in the texture is the current offset.
            font->characters[c].pos = currentOffset + uvec2(1,1); // + 1 for margin.
            font->characters[c].size = uvec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            font->characters[c].bearing = ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            font->characters[c].advance = (ushort)(face->glyph->advance.x >> 6);
            // Shift the offset to the right. This will be the new right-most edge for this row.
            currentOffset.x += font->characters[c].size.x + 2; // + 2 for margin.
            // Take whichever is more right, the current width, or the right-most edge of this glyph.
            texDimensions.x = std::max(currentOffset.x, texDimensions.x);
            // Take whichever is more down, the current height, or the bottom edge of this glyph.
            texDimensions.y = std::max(currentOffset.y + font->characters[c].size.y + 2, texDimensions.y);

            font->maxDescent = (ushort)std::max((int)font->maxDescent,
                (int)font->characters[c].size.y - (int)font->characters[c].bearing.y);
            // Increment the order index.
            orderIndex++;
            // If we've started a new row, shift to the left, and below the current height.
            if(orderIndex % FONT_CHAR_ROW == 0) {
                currentOffset.x = 0;
                currentOffset.y = texDimensions.y;
            }
        }
    }

    if(texDimensions.x == 0 || texDimensions.y == 0) {
        fprintf(stderr, "Something went horribly wrong with Font loading.\n");
        delete font;
        FT_Done_Face(face);
        FT_Done_FreeType(freeType);
        return make_pair(nullptr, nullptr);
    }

    font->sourceSize = uvec2(texDimensions.x, texDimensions.y);
    uchar* textureData = new uchar[texDimensions.x * texDimensions.y];

    for(uint c = 0; c < FONT_CHAR_COUNT; c++) {
        if(!font->characters[c].valid) {
            continue;
        }

        if(FT_Load_Char(face, c + FONT_CHAR_START, FT_LOAD_RENDER)) {
            font->characters[c].valid = false;
            continue;
        }

        uvec2 start = font->characters[c].pos;
        uvec2 end = start + font->characters[c].size;

        // Clear out the top margin.
        for(uint x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + (start.y - 1) * texDimensions.x] = 0;
        }
        for(uint y = start.y; y < end.y; y++) {
            // Clear out left margin.
            textureData[start.x - 1 + y * texDimensions.x] = 0;
            // Fill in this row of the texture with the bitmap.
            for(uint x = start.x; x < end.x; x++) {
                uchar cc = face->glyph->bitmap.buffer[
                    (x - start.x) + (y - start.y) * font->characters[c].size.x];
                textureData[x + y * texDimensions.x] = cc;
            }
            textureData[end.x + y * texDimensions.x] = 0;
        }
        // Clear out the bottom margin.
        for(uint x = start.x - 1; x < end.x + 1; x++) {
            textureData[x + end.y * texDimensions.x] = 0;
        }
    }

    // Build the texture.
    Texture* texture = new Texture();
    texture->fromGreyscale(textureData, texDimensions.x, texDimensions.y);

    FT_Done_Face(face);
    FT_Done_FreeType(freeType);

    return make_pair(font, texture);
}

Assimp::Importer gImporter;

string toLower(const string& str)
{
    string result;
    result.reserve(str.size() + 1);
    for(char c : str)
    {
        result += tolower(c);
    }
    return result;
}

// trim from left
inline string& ltrim(string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline string& rtrim(string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline string& trim(string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

void processResourceCommand(vector<string> command)
{
    string cmdType = toLower(command[0]);
    if(cmdType == "mesh")
    {
        string fileName = command[1];
        vector<string> meshNames;
        hash_map<string, string> meshFiles;
        if(command.size() % 2 != 0) {
            cerr << "Mesh command improperly formatted." << endl;
            return;
        }
        for(int i = 2; i < command.size(); i += 2) {
            meshNames.push_back(trim(command[i]));
            meshFiles.insert(make_pair(meshNames.back(), trim(command[i + 1])));
        }
        for(pair<Mesh*, string> res : extractMeshes(fileName, meshNames, meshFiles, gImporter))
        {
            if(!res.first->save(res.second)) {
                throw "Failed to save mesh";
            }
            delete res.first;
        }
    }
    else if(cmdType == "texture")
    {
        if(command.size() != 4) {
            cerr << "Invalid texture command: 'texture <format> <file> <outFile>'" << endl;
            return;
        }

        string format = command[1];
        string file = command[2];

        Texture* tex = nullptr;
        if(format == "png") {
            tex = loadPNGTexture(file);
        } else {
            cerr << "Bad format: '" << format << "'. Recognized formats: png" << endl;
            return;
        }
        
        string outFile = trim(command[3]);
        if(!tex || !tex->save(outFile)) {
            throw "Failed to load/save texture";
        }
        if(tex) {
            delete tex;
        }
    }
    else if(cmdType == "font") {
        if(command.size() != 4 && command.size() != 5) {
            cerr << "Invalid font command: 'font <source ttf> <font size> <outFile w/o extension> [face index]'" << endl;
            return;
        }
        uint size = stoi(command[2]);
        uint faceIndex = command.size() == 5 ? stoi(command[4]) : 0;
        pair<Font*, Texture*> font = loadFont(command[1], faceIndex, size);
        if(!font.first || !font.first->save(command[3] + ".fnt")
            || !font.second || !font.second->save(command[3] + ".tpk")) {
            throw "Failed to load/save font.";
        }
        if(font.first) {
            delete font.first;
        }
        if(font.second) {
            delete font.second;
        }
    }
    else
    {
        cerr << "Invalid command: " << cmdType << endl;
        return;
    }
}

vector<string> tokenize(string command)
{
    vector<string> tokens;
    while(!command.empty()) {
        if(command[0] == '"' || command[0] == '\'') {
            char end = command[0];
            string token = "";
            int i;
            for(i = 1; i < command.size(); i++) {
                if(command[i] == '\\') {
                    i++;
                    token += command[i];
                } else if(command[i] == end) {
                    break;
                } else {
                    token += command[i];
                }
            }
            if(i == command.size()) {
                fprintf(stderr, "Token not terminated: %s\n", command.c_str());
                throw 1;
            }
            tokens.push_back(token);
            if(i + 1 == command.size()) {
                command = "";
            } else {
                command = command.substr(i + 1);
            }
            trim(command);
        } else {
            size_t mid = command.find_first_of(" \t\n\r\f\v");
            tokens.push_back(command.substr(0, mid));
            if(mid == string::npos) {
                command = "";
            } else {
                command = command.substr(mid);
            }
            trim(command);
        }
    }
    return tokens;
}

void processInput()
{
    for(string command; getline(cin, command); )
    {
        trim(command);
        if(command.empty() || command[0] == '#') {
            continue;
        }
        processResourceCommand(tokenize(command));
    }
}

int main(int argc, char** argv)
{
    processInput();

    return 0;
}
