
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

void ConvexHull::addPoint(const vec3& point)
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

bool ConvexHull::load(shared_ptr<Resource::BuildData> data)
{
    shared_ptr<Mesh> sourceMesh = sourceMeshRef.resolve(Immediate); // Make sure this is loaded.
    assert(sourceMesh);
    
    for(uint i = 0; i < sourceMesh->vertCount; i++) {
        addPoint(sourceMesh->vertData[i].position);
    }
    completeHull();

    return true;
}

shared_ptr<ConvexHull> ConvexHull::build(shared_ptr<BuildData> data)
{
    shared_ptr<ConvexHull> mesh(new ConvexHull());
    mesh->sourceMeshRef = data->sourceMesh;
    return mesh;
}

shared_ptr<ConvexHull::BuildData> ConvexHull::createAssetData(uint sourceMesh)
{
    shared_ptr<BuildData> data = make_shared<BuildData>();
    data->sourceMesh = sourceMesh;
    return data;
}
