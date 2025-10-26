#pragma once

// #include <memory>
// #include <unordered_map>
// #include <any>
// #include <typeindex>

#include "components.hpp"
#include "ecs/componentMetaInfo.hpp"
#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systems.hpp"

#include <cstddef>

// Sparse set ECS implementation.

namespace gxe {

class ecs {
public:
    ecs() = default;
    ~ecs() = default;

    entity& createEntity(){ // Create the entity within the _entities vector.
        entityid id = _idManager.createEntity();

        if(id >= _entities.size()){ // Resize the vector and fill with null entities.
            _entities.resize(id + INITIAL_SPARSE_SET_CAPACITY, entity(NULL_ID, nullptr));
        }
        
        _entities[id] = entity(id, this); 
        return _entities[id];
    }

    void destroyEntity(entityid id){
        _idManager.destroyEntity(id); // Free the ID
        entity& e = _entities[id];
        
        for(std::size_t i = 0; i < n_components; i++){
            if(e.signature().bits().test(i)){ // If this entity has component.
                removeComponentAtIndex(id, i);
            }
        }

        e.signature().resetAll();
        _entities[id] = entity(NULL_ID, nullptr); // Replace with null entity.
    }

    template<typename T>
    entity& addComponent(entityid id, const T& component) { // Return entity& for chaining
        getSet<T>()->insert(id, component);
        _entities[id].signature().set<T>();

        return _entities[id];
    }

    template<typename T>
    entity& removeComponent(entityid id) { // Return entity& for chaining
        getSet<T>()->remove(id);
        _entities[id].signature().reset<T>();

        return _entities[id];
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

        for(std::size_t i = 0; i < firstSet->size(); i++){
            entityid id = firstSet->getEntityId(i);

            if(hasComponents<Ts...>(id)){ // If the Entity has all the required components.
                func(id, getComponent<Ts>(id)...);
            }
        }
    }

private:
    template<typename T>
    std::size_t getSetSize() {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    template<typename First, typename ...Rest>
    sparseSet<First>* getFirstSet(){
        return getSet<First>();
    }

    template<typename ...Ts>
    std::size_t getSmallestSetSize() {
        return std::min({getSetSize<Ts>()...});
    }

    // Set for index methods
    template<std::size_t... Is>
    void removeComponentAtIndexImpl(entityid id, std::size_t index, std::index_sequence<Is...>){
        ((Is == index ? (getSet<ComponentAt<Is>>()->remove(id), 0) : 0), ...);
    }

    void removeComponentAtIndex(entityid id, std::size_t index){
        removeComponentAtIndexImpl(id, index, std::make_index_sequence<n_components>{});
    }

    std::vector<entity> _entities; // Vector of entities by indexed by their ID.

    idManager _idManager; // Manages ID assignment at creation and deletion of objects.
    components _activeComponents; // Sparse set component collection. access via std::get<sparseSet<T>*>(_activeComponents)
};

} // namespace gxe

#include "entities/entity.tpp" // Template implementations for Entit, this avoids circular dependencies.