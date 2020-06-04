
#pragma once

#include "std.h"
#include "resources/ResourceLoader.h"
#include "resources/Mesh.h"
#include <bullet/BulletCollision/CollisionShapes/btConvexHullShape.h>

class ConvexHull : public Resource
{
public:
    ConvexHull();
    virtual ~ConvexHull();

    void addPoint(const vec3& point);

    void completeHull();

    static shared_ptr<Resource> build(shared_ptr<Resource::BuildData> data) {
        shared_ptr<BuildData> buildData = dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        uint sourceMesh;
    };

    class btConvexHullShape* createHullInstance() const;

    static shared_ptr<BuildData> createAssetData(uint sourceMesh);
protected:
    ResourceRef<Mesh> sourceMeshRef;

    virtual vector<uint> getDependencies() override {
        return { sourceMeshRef };
    }
    virtual void resolveDependencies(ResolveMethod method) override {
        sourceMeshRef.resolve(method);
    }
    virtual bool load(shared_ptr<Resource::BuildData> data) override;

    class btConvexHullShape* shape = nullptr;

    friend class PhysicsSystem;
private:
    static shared_ptr<ConvexHull> build(shared_ptr<BuildData> data);
};
