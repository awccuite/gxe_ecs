#pragma once

// #include <memory>
// #include <unordered_map>
// #include <any>
// #include <typeindex>

#include "components.hpp"
#include "idManager.hpp"
#include "sparseSet.hpp"
#include "entity.hpp" // Inlucde method signatures for Entity.

#include <iostream>

// Sparse set ECS implementation.

namespace gxe {

class ecs {
public:
    // We don't need to register components as they live in the tuple and are created at compile time.    
    ecs() = default;
    ~ecs() = default;

    Entity createEntity(){
        EntityID id = _idManager.createEntity();

        std::cout << "Created entity " << id << "\n";

        return Entity(id, this);
    }

    template<typename T>
    void addComponent(EntityID id, const T& component) {
        getSet<T>()->insert(id, component);
    }

    template<typename T>
    void removeComponent(EntityID id) {
        getSet<T>()->remove(id);
    }

    template<typename T>
    T& getComponent(EntityID id) {
        return getSet<T>()->get(id);
    }

    template<typename T>
    const T& getComponent(EntityID id) const {
        return const_cast<ecs*>(this)->getSet<T>()->get(id);
    }

    template<typename ...Ts>
    bool hasComponents(EntityID id) const {
        return (getSet<Ts>()->has(id) && ...);
    }

    template<typename T>
    SparseSet<T>* getSet() { // Return the dense set component
        return &std::get<SparseSet<T>>(_activeComponents);
    }

    // Variadic templated forEach function that accepts a
    // lambda function for system functionality.
    template<typename ...Ts, typename Func>
    void forEach(Func&& func) {

    }

private:
    IdManager _idManager; // Manages ID assignment at creation and deletion of objects.
    Components _activeComponents;
};

} // namespace gxe

#include "entity.tpp" // Template implementations for Entit, this avoids circular dependencies.