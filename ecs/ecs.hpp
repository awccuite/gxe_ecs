#pragma once

// #include <memory>
// #include <unordered_map>
// #include <any>
// #include <typeindex>

#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systems.hpp"

#include <cstddef>

// Sparse set ECS implementation.

namespace gxe {

class ecs {
public:
    ecs() {
        _entities.reserve(INITIAL_SPARSE_SET_CAPACITY);
    };
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
        auto& sig = _entities[id].signature();

        for(size_t i = sig.firstSet(); i < n_components; i = sig.nextSet(i)){
            removeComponentAtIndex(id, i);
        }

        sig.resetAll();
        _entities[id] = entity(NULL_ID, nullptr);
        _idManager.destroyEntity(id);

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

    template<typename T>
    const sparseSet<T>* getSet() const {
        return &std::get<sparseSet<T>>(_activeComponents);
    }

    // Variadic templated forEach function that accepts a
    // lambda function for system functionality. 
    // Applies to entites that only have the specified component signature.
    template<typename ...Ts, typename Func>
    void forEachEntityWith(Func&& func) {
        auto* smallestSet = getSmallestSet<Ts...>();
        if(!smallestSet) { return; };

        for(auto& entry : smallestSet->data()){
            if(hasComponents<Ts...>(entry.id)){ // If the Entity has all the required components.
                func(entry.id, getComponent<Ts>(entry.id)...);
            }
        }
    }

private:
    template<typename T>
    std::size_t getSetSize() {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    // Helper: get the index of the smallest set among Ts...
    template<typename ...Ts>
    std::size_t getSmallestSetIndex() const {
        std::array<std::size_t, sizeof...(Ts)> sizes = { getSet<Ts>()->size()... };
        return static_cast<std::size_t>(
            std::distance(sizes.begin(), std::min_element(sizes.begin(), sizes.end()))
        );
    }

    template<typename ...Ts, std::size_t... Is>
    auto* getSmallestSetImpl(std::size_t idx, std::index_sequence<Is...>) {
        using ret_type = std::common_type_t<sparseSet<Ts>*...>;
        ret_type result = nullptr;
        ((idx == Is ? result = getSet<std::tuple_element_t<Is, std::tuple<Ts...>>>(), 0 : 0), ...);
        return result;
    }

    // Helper: get the concrete sparseSet<T> pointer for the smallest set
    template<typename ...Ts>
    auto* getSmallestSet() {
        constexpr std::size_t N = sizeof...(Ts);
        std::size_t idx = getSmallestSetIndex<Ts...>();
        return getSmallestSetImpl<Ts...>(idx, std::make_index_sequence<N>{});
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