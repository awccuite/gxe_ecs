#pragma once

#include "../types.hpp"

namespace gxe {

template<typename ...Components>
class ecs; // Forward declare ecs

template<typename ...Components>
class entity {
public:
    // Copy constructors
    entity(const entity&) = default;
    entity& operator=(const entity&) = default;

    // Move constructors
    entity(entity&&) = default;
    entity& operator=(entity&&) = default;

    entityid id() const { return _id; }

    // Add component to entity.
    template<typename T>
    entity<Components...>& addComponent(const T& component) {
        _ecs->template addComponent<T>(_id, component);
        return *this;
    }

    // Remove component of type t from entity (if it exists)
    template<typename T>
    entity<Components...>& removeComponent() {
        _ecs->template removeComponent<T>(_id);
        return *this;
    }

    template<typename T>
    T& getComponent() {
        return _ecs->template getComponent<T>(_id);
    }

    template<typename T>
    const T& getComponent() const {
        return _ecs->template getComponent<T>(_id);
    }

    template<typename ...Ts>
    bool hasComponents() const {
        return _ecs->template hasComponents<Ts...>(_id);
    }

private:
    friend class ecs<Components...>;
    entity(entityid id, ecs<Components...>* ecsInstance) : _id(id), _ecs(ecsInstance) {};

    entityid _id;
    ecs<Components...>* _ecs; // Pointer to ecs
};

} // namespace gxe