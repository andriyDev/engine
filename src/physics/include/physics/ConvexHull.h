
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

    void addPoint(const glm::vec3& point);

    void completeHull();

    static std::shared_ptr<Resource> build(std::shared_ptr<Resource::BuildData> data) {
        std::shared_ptr<BuildData> buildData = std::dynamic_pointer_cast<BuildData>(data);
        return build(buildData);
    }

    class BuildData : public Resource::BuildData
    {
    public:
        uint sourceMesh;
    };

    class btConvexHullShape* createHullInstance() const;

    static std::shared_ptr<BuildData> createAssetData(uint sourceMesh);
protected:
    ResourceRef<Mesh> sourceMeshRef;

    virtual std::vector<uint> getDependencies() override {
        return { sourceMeshRef };
    }
    virtual void resolveDependencies(ResolveMethod method) override {
        sourceMeshRef.resolve(method);
    }
    virtual bool load(std::shared_ptr<Resource::BuildData> data) override;

    class btConvexHullShape* shape = nullptr;

    friend class PhysicsSystem;
private:
    static std::shared_ptr<ConvexHull> build(std::shared_ptr<BuildData> data);
};
