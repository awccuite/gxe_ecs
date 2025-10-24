#pragma once

#include "ecs.hpp"

namespace gxe {

template<typename T>
Entity& Entity::addComponent(const T& component) {
    _ecs->addComponent(_id, component);
    return *this;
}

template<typename T>
Entity& Entity::removeComponent() {
    _ecs->removeComponent<T>(_id);
    return *this;
}

template<typename T>
T& Entity::getComponent() {
    return _ecs->getComponent<T>(_id);
}

template<typename T>
const T& Entity::getComponent() const {
    return _ecs->getComponent<T>(_id);
}

template<typename T>
bool Entity::hasComponent() const {
    return _ecs->hasComponent<T>(_id);
}

} // namespace gxe
