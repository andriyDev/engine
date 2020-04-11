
#include "renderer/RenderSystem.h"

#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "components/Transform.h"

#include "ComponentTypes.h"

void RenderSystem::frameTick(float delta, float tickPercent)
{
    
}

void RenderSystem::gameplayTick(float delta)
{
    Query<Transform*> transforms = getWorld()->queryComponents()
        .filter([](Component* C){ return C->getTypeId() == TRANSFORM_ID; })
        .cast<Transform*>();
    for(Transform* transform : transforms)
    {
        transform->previousTransform[0] = transform->previousTransform[1];
        transform->previousTransform[1] = transform->relativeTransform;
    }
}
