#include "scene.h"

ObjectID Scene::addObject(Object object) {
    ObjectID id = nextID++;
    objects.try_emplace(id, std::move(object));
    drawOrder.emplace_back(id);
    return id;
}

void Scene::removeObject(ObjectID id) {
    objects.erase(id);
    std::erase(drawOrder, id);
}
