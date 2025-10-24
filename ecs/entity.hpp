#pragma once

#include "components.hpp"

namespace gxe {

class ecs; // Forward declare ecs

class Entity {
public:
    // Prevent copying
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Allow moving
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    EntityID id() const { return _id; }

    template<typename T>
    Entity& addComponent(const T& component);

    template<typename T>
    Entity& removeComponent();

    template<typename T>
    T& getComponent();

    template<typename T>
    const T& getComponent() const;

    template<typename T>
    bool hasComponent() const;

private:
    friend class ecs;
    Entity(EntityID id, ecs* ecsInstance) : _id(id), _ecs(ecsInstance) {}

    EntityID _id;
    ecs* _ecs;
};

} // namespace gxe