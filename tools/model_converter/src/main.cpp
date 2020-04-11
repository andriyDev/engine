
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <sstream>

#include "std.h"

#include "Serializer.h"

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

#define USAGE "Usage: model_converter <file1>:<srcMeshName1.1>[/<outMeshName1.1>][:<srcMeshName1.2>...] ... <out_file>\n"

vector<pair<aiMesh*, string>> collectMeshes(int argc, char** argv, Assimp::Importer& importer)
{
    vector<pair<aiMesh*, string>> meshesToProcess;
    for(int i = 1; i < argc - 1; i++) {
        string conversion(argv[i]);
        vector<string> components = splitByDelimiter(conversion, ':');

        string fileName = components[0];
        map<string, string> inOutMeshNames;
        vector<string> meshNames;
        for(int j = 1; j < components.size(); j++) {
            vector<string> strPair = splitByDelimiter(components[j], '/');
            if(components.size() > 2) {
                printf(USAGE);
                return vector<pair<aiMesh*, string>>();
            }
            string inMesh = strPair[0];
            string outMesh = strPair.size() == 2 ? strPair[1] : inMesh;
            inOutMeshNames.insert(make_pair(inMesh, outMesh));
            meshNames.push_back(inMesh);
        }

        const aiScene* scene = importer.ReadFile(fileName,
            aiProcess_Triangulate | aiProcess_FlipUVs);
        
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
            meshesToProcess.push_back(make_pair(mesh, name));
        }
    }
    return move(meshesToProcess);
}

int main(int argc, char** argv)
{
    if(argc == 1) {
        printf(USAGE);
        return -1;
    }

    Assimp::Importer importer;
    string outFileName = argv[argc - 1];
    vector<pair<aiMesh*, string>> meshesToProcess = collectMeshes(argc, argv, importer);

    for(pair<aiMesh*, string> p : meshesToProcess) {

    }

    return 0;
}
