#pragma once

#include "object.h"

class ResourceManager;
using ObjectID = uint32_t;

class Scene {
public:
    Scene(const VulkanDevice& device, ResourceManager& resources);

    ObjectID createObject(Transform transform, std::string_view mesh, std::string_view texture);

    const Object& getObject(ObjectID id) const { return objects_.at(id); }
    Object& getObject(ObjectID id) { return objects_.at(id); }
    void removeObject(ObjectID id);

    std::unordered_map<ObjectID, Object>& getObjects() { return objects_; }
    const std::unordered_map<ObjectID, Object>& getObjects() const { return objects_; }
    std::vector<ObjectID>& getDrawOrder() { return drawOrder_; }
    const std::vector<ObjectID>& getDrawOrder() const { return drawOrder_; }

private:
    ObjectID addObject(Object object);

    const VulkanDevice& device_;
    ResourceManager& resources_;

    std::unordered_map<ObjectID, Object> objects_;
    std::vector<ObjectID> drawOrder_;

    ObjectID nextID_{0};
};
