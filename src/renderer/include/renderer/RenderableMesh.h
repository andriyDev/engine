
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

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        uint sourceMesh;
    };

    static shared_ptr<BuildData> createAssetData(uint sourceMesh);

protected:
    RenderableMesh() {}

    GLuint vao = 0;
    GLuint buffers[2] = {0, 0};
    uint bufferCount;
    uint indexCount;

    ResourceRef<Mesh> sourceMeshRef;

    virtual vector<uint> getDependencies() override {
        return { sourceMeshRef };
    }
    virtual void resolveDependencies(ResolveMethod method) override {
        sourceMeshRef.resolve(method);
    }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

private:
    static shared_ptr<RenderableMesh> build(shared_ptr<BuildData> data);
};
