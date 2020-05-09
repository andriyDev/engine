
#include "resources/Mesh.h"

Mesh::Mesh()
    : Resource((uint)RenderResources::Mesh)
{}

Mesh::~Mesh()
{
    clearData();
}

void makeBoxFace(Mesh::Vertex* data, glm::vec3 extents, glm::vec3 normal, bool vert)
{
    glm::vec3 tangent = -glm::normalize(glm::cross(normal, vert ? glm::vec3(0, 0, -1) : glm::vec3(0, 1, 0)));
    glm::vec3 bitangent = glm::cross(normal, tangent);
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
    data[0].colour = glm::vec4(1,1,1,1);
    data[1].colour = glm::vec4(1,1,1,1);
    data[2].colour = glm::vec4(1,1,1,1);
    data[3].colour = glm::vec4(1,1,1,1);
    data[0].texCoord = glm::vec2(0,0);
    data[1].texCoord = glm::vec2(1,0);
    data[2].texCoord = glm::vec2(1,1);
    data[3].texCoord = glm::vec2(0,1);
}

std::shared_ptr<Mesh> Mesh::makeBox(glm::vec3 extents)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->vertCount = 24;
    mesh->indexCount = 36;
    mesh->vertData = new Vertex[24];
    mesh->indexData = new uint[36];

    makeBoxFace(mesh->vertData    , glm::vec3(extents.x, extents.y, extents.z), glm::vec3(0,0,-1), false);
    makeBoxFace(mesh->vertData + 4, glm::vec3(extents.x, extents.y, extents.z), glm::vec3(0,0,1), false);

    makeBoxFace(mesh->vertData + 8, glm::vec3(extents.z, extents.y, extents.x), glm::vec3(-1,0,0), false);
    makeBoxFace(mesh->vertData + 12, glm::vec3(extents.z, extents.y, extents.x), glm::vec3(1,0,0), false);

    makeBoxFace(mesh->vertData + 16, glm::vec3(extents.x, extents.z, extents.y), glm::vec3(0,-1,0), true);
    makeBoxFace(mesh->vertData + 20, glm::vec3(extents.x, extents.z, extents.y), glm::vec3(0,1,0), true);

    for(int i = 0; i < 6; i++) {
        mesh->indexData[i*6    ] = i*4;
        mesh->indexData[i*6 + 1] = i*4 + 1;
        mesh->indexData[i*6 + 2] = i*4 + 2;
        mesh->indexData[i*6 + 3] = i*4 + 2;
        mesh->indexData[i*6 + 4] = i*4 + 3;
        mesh->indexData[i*6 + 5] = i*4;
    }
    mesh->state = Success;
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

void writeMesh(Serializer& ser, void* meshRaw)
{
    Mesh* mesh = static_cast<Mesh*>(meshRaw);
    write(ser, *mesh);
}

void* readMesh(Serializer& ser)
{
    Mesh* mesh = new Mesh();
    read(ser, *mesh);
    return mesh;
}

void readIntoMesh(Serializer& ser, void* meshRaw)
{
    Mesh* mesh = static_cast<Mesh*>(meshRaw);
    read(ser, *mesh);
}
