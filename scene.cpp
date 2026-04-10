#include "scene.h"

#include "resource_manager.h"
#include "vulkan_device.h"

Scene::Scene(const VulkanDevice& device, ResourceManager& resources) : device_(device), resources_(resources) {
}

ObjectID Scene::createObject(Transform transform, std::string_view mesh, std::string_view texture) {
    std::vector layouts(Config::MAX_FRAMES_IN_FLIGHT, resources_.getObjectLayout());
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = resources_.getPool(),
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };
    auto descriptorSets = device_.getDevice().allocateDescriptorSets(allocInfo);

    return addObject({
        device_, transform, resources_.getMesh(mesh),
        resources_.getTexture(texture), std::move(descriptorSets)
    });
}

ObjectID Scene::addObject(Object object) {
    ObjectID id = nextID_++;
    objects_.try_emplace(id, std::move(object));
    drawOrder_.emplace_back(id);
    return id;
}

void Scene::removeObject(ObjectID id) {
    objects_.erase(id);
    std::erase(drawOrder_, id);
}
