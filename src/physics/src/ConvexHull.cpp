
#include "physics/ConvexHull.h"

#include "physics/BulletUtil.h"

ConvexHull::ConvexHull()
{
    shape = new btConvexHullShape();
}

ConvexHull::~ConvexHull()
{
    delete shape;
}

void ConvexHull::addPoint(const glm::vec3& point)
{
    shape->addPoint(convert(point), false);
}

void ConvexHull::completeHull()
{
    shape->optimizeConvexHull();
    shape->recalcLocalAabb();
}

btConvexHullShape* ConvexHull::createHullInstance() const
{
    return new btConvexHullShape((btScalar*)shape->getUnscaledPoints(), shape->getNumPoints());
}

bool ConvexHull::load(std::shared_ptr<Resource::BuildData> data)
{
    std::shared_ptr<Mesh> sourceMesh = sourceMeshRef.resolve(Immediate); // Make sure this is loaded.
    assert(sourceMesh);
    
    for(uint i = 0; i < sourceMesh->vertCount; i++) {
        addPoint(sourceMesh->vertData[i].position);
    }
    completeHull();

    return true;
}

std::shared_ptr<ConvexHull> ConvexHull::build(std::shared_ptr<BuildData> data)
{
    std::shared_ptr<ConvexHull> mesh(new ConvexHull());
    mesh->sourceMeshRef = data->sourceMesh;
    return mesh;
}

std::shared_ptr<ConvexHull::BuildData> ConvexHull::createAssetData(uint sourceMesh)
{
    std::shared_ptr<BuildData> data = std::make_shared<BuildData>();
    data->sourceMesh = sourceMesh;
    return data;
}
