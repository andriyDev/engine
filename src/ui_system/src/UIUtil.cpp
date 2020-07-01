
#include "UIUtil.h"

UIUtil UIUtil::data;

UIUtil::~UIUtil()
{
    if(vertBuffer) {
        glDeleteBuffers(1, &vertBuffer);
    }
    if(rectangleMesh) {
        glDeleteVertexArrays(1, &rectangleMesh);
    }
}

void UIUtil::bindRectangle()
{
    if(data.rectangleMesh) {
        glBindVertexArray(data.rectangleMesh);
        return;
    }

    data.buildMesh(); // After this command, the VAO will be bound.
}

void UIUtil::buildMesh()
{
    glGenVertexArrays(1, &data.rectangleMesh);
    glBindVertexArray(data.rectangleMesh);
    glGenBuffers(1, &vertBuffer);

    static const float data[12] = {
        0, 0,
        0, 1,
        1, 1,
        1, 1,
        1, 0,
        0, 0
    };

    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void UIUtil::renderRectangle()
{
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIUtil::renderRectangles(uint rectangleCount)
{
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangleCount);
}
