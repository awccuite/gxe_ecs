#pragma once
#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systemManager.hpp"

#include <iostream>

// Sparse set ECS implementation.

namespace gxe {

class ecs {
public:
    ecs() {
        _entities.reserve(INITIAL_SPARSE_SET_CAPACITY);
    };
    ~ecs() = default;

    entity& createEntity() {
        entityid id = _idManager.createEntity();

        if(id >= _entities.size()) {
            _entities.resize(id + INITIAL_SPARSE_SET_CAPACITY, entity(NULL_ID, nullptr));
        }
        
        _entities[id] = entity(id, this); 
        return _entities[id];
    }

    void destroyEntity(entityid id) {
        auto& sig = _entities[id].signature();

        removeAllComponents(id);

        sig.resetAll();
        _entities[id] = entity(NULL_ID, nullptr);
        _idManager.destroyEntity(id);
    }

    template<typename T>
    entity& addComponent(entityid id, const T& component) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->insert(id, component);
        _entities[id].signature().set<T>();

        return _entities[id];
    }

    template<typename T>
    entity& removeComponent(entityid id) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->remove(id);
        _entities[id].signature().reset<T>();

        return _entities[id];
    }

    template<typename T>
    T& getComponent(entityid id) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return getSet<T>()->get(id);
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
        return &std::get<sparseSet<T>>(_activeComponents);
    }

    template<typename T>
    const sparseSet<T>* getSet() const {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return &std::get<sparseSet<T>>(_activeComponents);
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

    void printEntitySignature(entityid id) const {
        if(id < _entities.size() && _entities[id].id() != NULL_ID) {
            std::cout << "Entity " << id << ": " 
                      << _entities[id].signature().toString() << std::endl;
        }
    }

    std::size_t entityCount() const {
        return _idManager.entityCount();
    }

private:
    template<typename T>
    std::size_t getSetSize() const {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    // Get index of smallest set among Ts...
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

    template<typename ...Ts>
    auto* getSmallestSet() {
        constexpr std::size_t N = sizeof...(Ts);
        std::size_t idx = getSmallestSetIndex<Ts...>();
        return getSmallestSetImpl<Ts...>(idx, std::make_index_sequence<N>{});
    }

    // Component removal helpers
    template<std::size_t... Is>
    static void removeIfSet(entityid id, ComponentType type, ecs* self, std::index_sequence<Is...>) {
        ([&] {
            if (type == static_cast<ComponentType>(Is)) {
                self->getSet<ComponentAt<Is>>()->remove(id);
            }
        }(), ...);
    }

    void removeAllComponents(entityid id) {
        auto const& sig = _entities[id].signature();
        if(sig.empty()) return;
        
        sig.forEachSet([&](ComponentType type) {
            removeIfSet(id, type, this, std::make_index_sequence<n_components>{});
        });
    }

    std::vector<entity> _entities; // Vector of entities by indexed by their ID.
    idManager _idManager; // Manages ID assignment at creation and deletion of objects.
    components _activeComponents; // Sparse set component collection. access via std::get<sparseSet<T>*>(_activeComponents)

    systemManger _systemManager; // Manages execution of systems.
};

} // namespace gxe

#include "entities/entity.tpp" // Template implementations for Entit, this avoids circular dependencies.