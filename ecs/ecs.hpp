#pragma once

// #include <memory>
// #include <unordered_map>
// #include <any>
// #include <typeindex>

#include "components.hpp"
#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systems.hpp"

#include <cstddef>
#include <iostream>
#include <limits>

// Sparse set ECS implementation.

namespace gxe {

class ecs {
public:
    // We don't need to register components as they live in the tuple and are created at compile time.    
    ecs() = default;
    ~ecs() = default;

    entity createEntity(){
        return entity(_idManager.createEntity(), this);
    }

    template<typename T>
    void addComponent(entityid id, const T& component) {
        getSet<T>()->insert(id, component);
    }

    template<typename T>
    void removeComponent(entityid id) {
        getSet<T>()->remove(id);
    }

    template<typename T>
    T& getComponent(entityid id) {
        return getSet<T>()->get(id);
    }

    template<typename T>
    const T& getComponent(entityid id) const {
        return const_cast<ecs*>(this)->getSet<T>()->get(id);
    }

    template<typename ...Ts>
    bool hasComponents(entityid id) {
        return (getSet<Ts>()->has(id) && ...);
    }

    template<typename T>
    sparseSet<T>* getSet() {
        return &std::get<sparseSet<T>>(_activeComponents);
    }

    // Variadic templated forEach function that accepts a
    // lambda function for system functionality. 
    // Applies to entites that only have the specified component signature.
    template<typename ...Ts, typename Func>
    void forEachEntityWith(Func&& func) {
        auto* firstSet = getFirstSet<Ts...>();
        if(!firstSet) { return; };

        for(size_t i = 0; i < firstSet->size(); i++){
            entityid id = firstSet->getEntityId(i);

            if(hasComponents<Ts...>(id)){ // If the Entity has all the required components.
                func(id, getComponent<Ts>(id)...);
            }
        }
    }

private:
    template<typename T>
    size_t getSetSize() {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    template<typename First, typename ...Rest>
    sparseSet<First>* getFirstSet(){
        return getSet<First>();
    }

    template<typename ...Ts>
    size_t getSmallestSetSize() {
        return std::min({getSetSize<Ts>()...});
    }

    idManager _idManager; // Manages ID assignment at creation and deletion of objects.
    components _activeComponents;
};

} // namespace gxe

#include "entities/entity.tpp" // Template implementations for Entit, this avoids circular dependencies.