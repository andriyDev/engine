
#include "resources/Mesh.h"

Mesh::~Mesh()
{
    clearData();
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
