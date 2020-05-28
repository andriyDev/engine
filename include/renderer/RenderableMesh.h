
#pragma once

#include "std.h"

#include "resources/Mesh.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableMesh : public Resource
{
public:
    virtual ~RenderableMesh();

    void bind();
    void render();

    static std::shared_ptr<Resource> build(std::shared_ptr<void> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

protected:
    GLuint vao = 0;
    GLuint buffers[2] = {0, 0};
    uint bufferCount;
    uint indexCount;

    ResourceRef<Mesh> sourceMeshRef;

    virtual std::vector<uint> getDependencies() override {
        return { sourceMeshRef };
    }
    virtual void resolveDependencies() override {
        sourceMeshRef.resolve();
    }
    virtual bool load(std::shared_ptr<void> data) override;

private:
    struct BuildData
    {
        uint sourceMesh;
    };

    static std::shared_ptr<RenderableMesh> build(std::shared_ptr<BuildData> data);
};
