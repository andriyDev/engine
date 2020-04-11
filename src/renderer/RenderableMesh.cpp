
#include "renderer/RenderableMesh.h"

RenderableMesh::RenderableMesh(Mesh* mesh)
{
    bufferCount = 2;
    indexCount = mesh->indexCount;

    glGenVertexArrays(1, &vao);
    bind();
    glGenBuffers(bufferCount, buffers);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexCount * sizeof(uint), mesh->indexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
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
