
#include "core/Universe.h"
#include "core/Entity.h"
#include "core/Component.h"
#include "core/World.h"

shared_ptr<World> Universe::addWorld()
{
    shared_ptr<World> ptr = make_shared<World>();
    addWorld(ptr);
    return ptr;
}

void Universe::addWorld(shared_ptr<World> world)
{
    worlds.push_back(world);
}

shared_ptr<World> Universe::removeWorld(weak_ptr<World> world)
{
    shared_ptr<World> wptr = world.lock();
    if(wptr) {
        worlds.erase(find(worlds.begin(), worlds.end(), wptr));
    }
    return wptr;
}


void Universe::tick(float deltaTime)
{
    totalTime += deltaTime;
    float gameplayDelta = 1.0f / gameplayRate;
    float remainingGameplayTime = totalTime - gameplayTime;
    int desiredGameplayFrames = (int)(remainingGameplayTime * gameplayRate);
    int issuedGameplayFrames = desiredGameplayFrames < maxGameplayTicksPerFrame
        ? desiredGameplayFrames : maxGameplayTicksPerFrame;

    for(int i = 0; i < issuedGameplayFrames; i++) {
        gameplayTime += gameplayDelta;
        for(auto p : worlds) {
            p->gameplayTick(gameplayDelta);
        }
    }

    float currentSkippedTime = (desiredGameplayFrames - issuedGameplayFrames) * gameplayDelta;
    skippedTime += currentSkippedTime;
    gameplayTime += currentSkippedTime;

    for(auto p : worlds) {
        p->frameTick(deltaTime);
    }
}
