#pragma once

#include "../components.hpp"
#include <bitset>

namespace gxe {

class ecs; // Forward declare ecs

// Entities need a signature associated with them, that
// lets us check what components they have. A signature is a bitset.

class entity {
public:
    // Copy constructors
    entity(const entity&) = default;
    entity& operator=(const entity&) = default;

    // Move constructors
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
    entity(entityid id, ecs* ecsInstance) : _id(id), _ecs(ecsInstance) {};

    entityid _id;
    ecs* _ecs;
    componentSignature _signature;
};

} // namespace gxe