
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "std.h"
#include "resources/Mesh.h"

#include "utility/Serializer.h"
#include "utility/Package.h"

using namespace std;

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

#define USAGE "Usage: model_converter <file1>,<srcMeshName1.1>[/<outMeshName1.1>][,<srcMeshName1.2>...] ... <out_file>\n"

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

vector<pair<Mesh*, string>> collectMeshes(int argc, char** argv, Assimp::Importer& importer)
{
    vector<pair<Mesh*, string>> meshesToProcess;
    for(int i = 1; i < argc - 1; i++) {
        string conversion(argv[i]);
        vector<string> components = splitByDelimiter(conversion, ',');

        string fileName = components[0];
        map<string, string> inOutMeshNames;
        vector<string> meshNames;
        for(int j = 1; j < components.size(); j++) {
            vector<string> strPair = splitByDelimiter(components[j], '/');
            if(components.size() > 2) {
                printf(USAGE);
                return vector<pair<Mesh*, string>>();
            }
            string inMesh = strPair[0];
            string outMesh = strPair.size() == 2 ? strPair[1] : inMesh;
            inOutMeshNames.insert(make_pair(inMesh, outMesh));
            meshNames.push_back(inMesh);
        }

        const aiScene* scene = importer.ReadFile(fileName,
            aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_CalcTangentSpace
            | aiProcess_GenUVCoords);
        
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "Conversion error: " << importer.GetErrorString() << endl;
            continue;
        }

        string storedName = field(fileName, '/', -1);
        storedName = storedName.substr(0, storedName.find_last_of('.'));

        vector<aiMesh*> meshes;
        findMeshes(scene->mRootNode, scene, meshNames, meshes);
        for(aiMesh* mesh : meshes) {
            string name;
            auto it = inOutMeshNames.find(mesh->mName.C_Str());
            name = it == inOutMeshNames.end() ? storedName : it->second;
            meshesToProcess.push_back(make_pair(convertMesh(mesh), name));
        }

        importer.FreeScene();
    }
    return meshesToProcess;
}

map<uint, pair<WriteFcn, ReadFcn>> parsers = {
    {RESOURCE_MESH, make_pair(writeMesh, readMesh)}
};

int main(int argc, char** argv)
{
    if(argc == 1) {
        printf(USAGE);
        return -1;
    }

    Assimp::Importer importer;
    vector<pair<Mesh*, string>> meshesToProcess = collectMeshes(argc, argv, importer);
    
    string outFileName = argv[argc - 1];
    ofstream file(outFileName);
    Package pack(Serializer(&file), &parsers);

    for(pair<Mesh*, string> p : meshesToProcess) {
        pack.addResource(p.second, RESOURCE_MESH, p.first);
    }
    pack.savePackage();
    for(pair<Mesh*, string> p : meshesToProcess) {
        delete p.first;
    }
    file.close();

    return 0;
}
