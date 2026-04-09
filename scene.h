#pragma once

#include "object.h"

using ObjectID = uint32_t;

class Scene {
public:
    ObjectID addObject(Object object);

    const Object& getObject(ObjectID id) const { return objects.at(id); }
    Object& getObject(ObjectID id) { return objects.at(id); }
    void removeObject(ObjectID id);

    std::unordered_map<ObjectID, Object>& getObjects() { return objects; }
    const std::unordered_map<ObjectID, Object>& getObjects() const { return objects; }
    std::vector<ObjectID>& getDrawOrder() { return drawOrder; }
    const std::vector<ObjectID>& getDrawOrder() const { return drawOrder; }

private:
    std::unordered_map<ObjectID, Object> objects;
    std::vector<ObjectID> drawOrder;

    ObjectID nextID{0};
};
