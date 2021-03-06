
#include "renderer/RenderableMesh.h"

RenderableMesh::RenderableMesh(ResourceRef<Mesh> sourceMesh)
    : sourceMeshRef(sourceMesh)
{
    resolveDependencies(Immediate);
    if(!load(nullptr)) {
        throw "Failed to create RenderableMesh!";
    }
}

RenderableMesh::~RenderableMesh()
{
    glDeleteBuffers(bufferCount, buffers);
    glDeleteVertexArrays(1, &vao);
}

void RenderableMesh::bind()
{
    glBindVertexArray(vao);
}

void RenderableMesh::render()
{
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
}

bool RenderableMesh::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Mesh> sourceMesh = sourceMeshRef.resolve(Immediate); // Make sure this is loaded.
    assert(sourceMesh);
    
    bufferCount = 2;
    indexCount = sourceMesh->indexCount;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(bufferCount, buffers);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sourceMesh->indexCount * sizeof(uint), sourceMesh->indexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sourceMesh->vertCount * sizeof(Mesh::Vertex), sourceMesh->vertData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, position));
    glEnableVertexAttribArray(1); // Colour
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, colour));
    glEnableVertexAttribArray(2); // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, texCoord));
    glEnableVertexAttribArray(3); // Normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, normal));
    glEnableVertexAttribArray(4); // Tangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, tangent));
    glEnableVertexAttribArray(5); // Bitangent
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, bitangent));

    sourceMeshRef = ResourceRef<Mesh>();
    return true;
}

shared_ptr<RenderableMesh> RenderableMesh::build(shared_ptr<BuildData> data)
{
    shared_ptr<RenderableMesh> mesh(new RenderableMesh());
    mesh->sourceMeshRef = data->sourceMesh;
    return mesh;
}

shared_ptr<RenderableMesh::BuildData> RenderableMesh::createAssetData(uint sourceMesh)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->sourceMesh = sourceMesh;
    return data;
}
