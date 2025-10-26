#pragma once

#include "../ecs.hpp"

namespace gxe {

template<typename T>
entity& entity::addComponent(T& component) {
    _ecs->addComponent(_id, std::forward<T>(component));
    return *this;
}

template<typename T>
entity& entity::removeComponent() {
    _ecs->removeComponent<T>(_id);
    return *this;
}

template<typename T>
T& entity::getComponent() {
    return _ecs->getComponent<T>(_id);
}

template<typename T>
const T& entity::getComponent() const {
    return _ecs->getComponent<T>(_id);
}

template<typename ...Ts>
bool entity::hasComponents() const {
    return (_ecs->hasComponents<Ts>(_id) && ...);
}

} // namespace gxe
