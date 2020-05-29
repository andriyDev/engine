
#pragma once

#include "std.h"

#include "resources/Mesh.h"

#define GLEW_STATIC
#include <GL/glew.h>

class RenderableMesh : public Resource
{
public:
    RenderableMesh(ResourceRef<Mesh> sourceMesh);
    virtual ~RenderableMesh();

    void bind();
    void render();

    static std::shared_ptr<Resource> build(std::shared_ptr<Resource::BuildData> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        uint sourceMesh;
    };

    static std::shared_ptr<BuildData> createAssetData(uint sourceMesh);

protected:
    RenderableMesh() {}

    GLuint vao = 0;
    GLuint buffers[2] = {0, 0};
    uint bufferCount;
    uint indexCount;

    ResourceRef<Mesh> sourceMeshRef;

    virtual std::vector<uint> getDependencies() override {
        return { sourceMeshRef };
    }
    virtual void resolveDependencies(ResolveMethod method) override {
        sourceMeshRef.resolve(method);
    }
    virtual bool load(std::shared_ptr<Resource::BuildData> data) override;

private:
    static std::shared_ptr<RenderableMesh> build(std::shared_ptr<BuildData> data);
};
