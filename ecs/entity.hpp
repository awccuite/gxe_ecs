#pragma once

#include "components.hpp"

namespace gxe {

class ecs; // Forward declare ecs

class entity {
public:
    // Prevent copying
    entity(const entity&) = delete;
    entity& operator=(const entity&) = delete;

    // Allow moving
    entity(entity&&) = default;
    entity& operator=(entity&&) = default;

    entityid id() const { return _id; }

    template<typename T>
    entity& addComponent(const T& component);

    template<typename T>
    entity& removeComponent();

    template<typename T>
    T& getComponent();

    template<typename T>
    const T& getComponent() const;

    template<typename ...Ts>
    bool hasComponents() const;

private:
    friend class ecs;
    entity(entityid id, ecs* ecsInstance) : _id(id), _ecs(ecsInstance) {}

    entityid _id;
    ecs* _ecs;
};

} // namespace gxe