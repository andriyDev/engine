
#include "renderer/RenderableMesh.h"

RenderableMesh::RenderableMesh()
    : Resource((uint)RenderResources::RenderableMesh)
{}

RenderableMesh::~RenderableMesh()
{
    if(state == Resource::Success) {
        glDeleteBuffers(bufferCount, buffers);
        glDeleteVertexArrays(1, &vao);
    }
}

void RenderableMesh::bind()
{
    if(state == Resource::Success) {
        glBindVertexArray(vao);
    }
}

void RenderableMesh::render()
{
    if(state == Resource::Success) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    }
}

std::shared_ptr<Resource> RenderableMeshBuilder::construct()
{
    return std::make_shared<RenderableMesh>();
}

void RenderableMeshBuilder::init()
{
    addDependency(sourceMesh);
}

void RenderableMeshBuilder::startBuild()
{
    std::shared_ptr<RenderableMesh> outMesh = getResource<RenderableMesh>();
    std::shared_ptr<Mesh> mesh = getDependency<Mesh>(sourceMesh, (uint)RenderResources::Mesh);
    if(!mesh) {
        throw "Bad Mesh!";
    }

    outMesh->bufferCount = 2;
    outMesh->indexCount = mesh->indexCount;

    glGenVertexArrays(1, &outMesh->vao);
    glBindVertexArray(outMesh->vao);
    glGenBuffers(outMesh->bufferCount, outMesh->buffers);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outMesh->buffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexCount * sizeof(uint), mesh->indexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, outMesh->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertCount * sizeof(Mesh::Vertex), mesh->vertData, GL_STATIC_DRAW);

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

    outMesh->state = Resource::Success;
}
