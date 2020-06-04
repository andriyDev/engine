
#include "resources/Mesh.h"

#include "utility/Serializer.h"

Mesh::~Mesh()
{
    clearData();
}

void makeBoxFace(Mesh::Vertex* data, vec3 extents, vec3 normal, bool vert)
{
    vec3 tangent = -normalize(cross(normal, vert ? vec3(0, 0, -1) : vec3(0, 1, 0)));
    vec3 bitangent = cross(normal, tangent);
    data[0].position = normal * extents.z - tangent * extents.x - bitangent * extents.y;
    data[1].position = normal * extents.z + tangent * extents.x - bitangent * extents.y;
    data[2].position = normal * extents.z + tangent * extents.x + bitangent * extents.y;
    data[3].position = normal * extents.z - tangent * extents.x + bitangent * extents.y;
    data[0].normal = normal;
    data[1].normal = normal;
    data[2].normal = normal;
    data[3].normal = normal;
    data[0].tangent = tangent;
    data[1].tangent = tangent;
    data[2].tangent = tangent;
    data[3].tangent = tangent;
    data[0].bitangent = bitangent;
    data[1].bitangent = bitangent;
    data[2].bitangent = bitangent;
    data[3].bitangent = bitangent;
    data[0].colour = vec4(1,1,1,1);
    data[1].colour = vec4(1,1,1,1);
    data[2].colour = vec4(1,1,1,1);
    data[3].colour = vec4(1,1,1,1);
    data[0].texCoord = vec2(0,0);
    data[1].texCoord = vec2(1,0);
    data[2].texCoord = vec2(1,1);
    data[3].texCoord = vec2(0,1);
}

shared_ptr<Mesh> Mesh::makeBox(vec3 extents)
{
    shared_ptr<Mesh> mesh = make_shared<Mesh>();
    mesh->vertCount = 24;
    mesh->indexCount = 36;
    mesh->vertData = new Vertex[24];
    mesh->indexData = new uint[36];

    makeBoxFace(mesh->vertData    , vec3(extents.x, extents.y, extents.z), vec3(0,0,-1), false);
    makeBoxFace(mesh->vertData + 4, vec3(extents.x, extents.y, extents.z), vec3(0,0,1), false);

    makeBoxFace(mesh->vertData + 8, vec3(extents.z, extents.y, extents.x), vec3(-1,0,0), false);
    makeBoxFace(mesh->vertData + 12, vec3(extents.z, extents.y, extents.x), vec3(1,0,0), false);

    makeBoxFace(mesh->vertData + 16, vec3(extents.x, extents.z, extents.y), vec3(0,-1,0), true);
    makeBoxFace(mesh->vertData + 20, vec3(extents.x, extents.z, extents.y), vec3(0,1,0), true);

    for(int i = 0; i < 6; i++) {
        mesh->indexData[i*6    ] = i*4;
        mesh->indexData[i*6 + 1] = i*4 + 1;
        mesh->indexData[i*6 + 2] = i*4 + 2;
        mesh->indexData[i*6 + 3] = i*4 + 2;
        mesh->indexData[i*6 + 4] = i*4 + 3;
        mesh->indexData[i*6 + 5] = i*4;
    }
    return mesh;
}

void Mesh::clearData()
{
    delete[] vertData;
    delete[] indexData;
    vertData = nullptr;
    indexData = nullptr;
}

template<>
void write(Serializer& ser, const Mesh::Vertex& vertex)
{
    write_array<uchar>(ser, (float*)&vertex, sizeof(Mesh::Vertex) / sizeof(float), true);
}

template<>
void read(Serializer& ser, Mesh::Vertex& vertex)
{
    read_array<uchar>(ser, (float*)&vertex, sizeof(Mesh::Vertex) / sizeof(float), true);
}

template<>
void write(Serializer& ser, const Mesh& mesh)
{
    write_array(ser, mesh.vertData, mesh.vertCount);
    write_array(ser, mesh.indexData, mesh.indexCount);
}

template<>
void read(Serializer& ser, Mesh& mesh)
{
    read_array_alloc(ser, mesh.vertData, mesh.vertCount);
    read_array_alloc(ser, mesh.indexData, mesh.indexCount);
}

void Mesh::loadFromFile(ifstream& file)
{
    Serializer ser(&file);
    read(ser, *this);
}

void Mesh::saveToFile(ofstream& file)
{
    Serializer ser(&file);
    write(ser, *this);
}
