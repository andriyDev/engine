
#pragma once

#include "std.h"
#include <algorithm>
#include <functional>

class Event
{
public:
    typedef std::function<void(void*)> EventFcn;
    uint addFunction(EventFcn f, void* data) {
        uint fcnId = rand(); // TODO: Better random id gen.
        fcns.insert(std::make_pair(fcnId, std::make_pair(data, f)));
        return fcnId; }
    void removeFunction(uint fcnId) { fcns.erase(fcns.find(fcnId)); }
    void clearFunctions() { fcns.clear(); }
    void dispatch() {
        for(auto p : fcns) { p.second.second(p.second.first); }
    }
private:
    std::map<uint, std::pair<void*, EventFcn>> fcns;
};

template<typename T>
class ParamEvent
{
public:
    typedef std::function<void(void*, T)> EventFcn;
    uint addFunction(EventFcn f, void* data) {
        uint fcnId = rand(); // TODO: Better random id gen.
        fcns.insert(std::make_pair(fcnId, std::make_pair(data, f)));
        return fcnId; }
    void removeFunction(uint fcnId) { fcns.erase(fcns.find(fcnId)); }
    void clearFunctions() { fcns.clear(); }
    void dispatch(T params) {
        for(auto p : fcns) { p.second.second(p.second.first, params); }
    }
private:
    std::map<uint, std::pair<void*, EventFcn>> fcns;
};
