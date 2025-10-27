#pragma once
#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systemManager.hpp"

#include <iostream>

// Sparse set ECS implementation.

// TODO:
// REWORK ecs to handle templated constructor ecs ecs<...components>();

namespace gxe {

template<typename ...Components>
class ecs {
// ECS templating logic, FUN :D
    static constexpr size_t N_COMPONENTS = sizeof...(Components);
    using ComponentSets = std::tuple<sparseSet<Components>...>;

public:
    ecs() {
        _entities.reserve(INITIAL_SPARSE_SET_CAPACITY);
    };
    ~ecs() = default;

    entity<Components...>& createEntity() {
        entityid id = _idManager.createEntity();
        std::cout << "Created entity " << static_cast<uint32_t>(id) << std::endl;

        if(id >= _entities.size()) {
            _entities.resize(id + INITIAL_SPARSE_SET_CAPACITY, entity<Components...>(NULL_ID, nullptr));
        }
        
        _entities[id] = entity<Components...>(id, this); 
        return _entities[id];
    }

    template<typename T>
    entity<Components...>& addComponent(entityid id, const T& component) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->insert(id, component);

        return _entities[id];
    }

    template<typename T>
    entity<Components...>& removeComponent(entityid id) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->remove(id);

        return _entities[id];
    }

    template<typename T>
    const T& getComponent(entityid id) const {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return const_cast<ecs*>(this)->getSet<T>()->get(id);
    }

    template<typename ...Ts>
    bool hasComponents(entityid id) {
        static_assert((IsComponent<Ts> && ...), "All types must be registered components");
        return (getSet<Ts>()->has(id) && ...);
    }

    template<typename T>
    sparseSet<T>* getSet() {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return &std::get<sparseSet<T>>(_componentSets);
    }

    template<typename T>
    const sparseSet<T>* getSet() const {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return &std::get<sparseSet<T>>(_componentSets);
    }

    // Iterate over each entity with the specified components
    template<typename ...Ts, typename Func>
    void forEachEntityWith(Func&& func) {
        static_assert((IsComponent<Ts> && ...), "All types must be registered components");
        
        auto* smallestSet = getSmallestSet<Ts...>();
        if(!smallestSet) return;

        // Prefetch components for better cache performance
        constexpr bool enablePrefetch = sizeof...(Ts) > 2;
        for(auto& entry : smallestSet->data()) {
            if(hasComponents<Ts...>(entry.id)) {
                if constexpr(enablePrefetch) {
                    (getSet<Ts>()->prefetch(entry.id), ...);
                }
                func(entry.id, getComponent<Ts>(entry.id)...);
            }
        }
    }

    std::size_t entityCount() const {
        return _idManager.entityCount();
    }

private:
    template<typename T>
    static constexpr bool IsComponent = (std::is_same_v<T, Components> || ...);

    template<typename T>
    std::size_t getSetSize() const {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    template<typename First, typename ...Rest>
    auto* getSmallestSet() {
        if constexpr (sizeof...(Rest) == 0) {
            return getSet<First>();
        } else {
            auto* firstSet = getSet<First>();
            auto* restSmallest = getSmallestSet<Rest...>();
            
            return (firstSet->size() <= restSmallest->size()) ? firstSet : restSmallest;
        }
    }

    std::vector<entity<Components...>> _entities; // Vector of entities by indexed by their ID.
    idManager _idManager; // Manages ID assignment at creation and deletion of objects.

    ComponentSets _componentSets;
};

} // namespace gxe