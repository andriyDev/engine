
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

#define USAGE "Usage: model_converter <out_file>\n"

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

vector<pair<void*, string>> extractMeshes(const string& fileName,
    vector<string> desiredMeshes, map<string, string> meshNameMap, Assimp::Importer& importer)
{
    vector<pair<void*, string>> resources;

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

map<uint, pair<WriteFcn, ReadFcn>> parsers = {
    {RESOURCE_MESH, make_pair(writeMesh, readMesh)},
    {RESOURCE_SHADER, make_pair(writeShader, readShader)}
};
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
inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

void processResourceCommand(vector<string> command, Package& pkg)
{
    string cmdType = toLower(command[0]);
    if(cmdType == "mesh")
    {
        string fileName = command[1];
        vector<string> meshNames;
        map<string, string> meshNameMap;
        for(int i = 2; i < command.size(); i++) {
            vector<string> split = splitByDelimiter(command[i], '/');
            if(split.size() > 2) {
                cerr << "Mesh command improperly formatted." << endl;
                return;
            }
            meshNames.push_back(trim(split[0]));
            meshNameMap.insert(make_pair(split[0], trim(split[split.size() == 2 ? 1 : 0])));
        }
        for(pair<void*, string> res : extractMeshes(fileName, meshNames, meshNameMap, gImporter))
        {
            pkg.addResource(res.second, RESOURCE_MESH, res.first);
        }
    }
    else if(cmdType == "shader")
    {
        Shader* shader = new Shader();
        ifstream CodeFile(command[1]);
        if(CodeFile.is_open()) {
            stringstream ss;
            ss << CodeFile.rdbuf();
            shader->code = ss.str();
            CodeFile.close();
        } else {
            cerr << "Cannot open shader code file." << endl;
            delete shader;
            return;
        }
        pkg.addResource(trim(command[2]), RESOURCE_SHADER, shader);
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

void processInput(Package& pkg)
{
    for(string command; getline(cin, command); )
    {
        trim(command);
        if(command.empty() || command[0] == '#') {
            continue;
        }
        processResourceCommand(tokenize(command), pkg);
    }
}

int main(int argc, char** argv)
{
    if(argc == 1) {
        printf(USAGE);
        return -1;
    }
    
    string outFileName = argv[argc - 1];
    ofstream file(outFileName);
    Package pack(Serializer(&file), &parsers);

    processInput(pack);

    pack.savePackage();
    pack.freeResources();
    file.close();

    return 0;
}
