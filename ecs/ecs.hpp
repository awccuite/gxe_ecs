#pragma once
#include "entities/idManager.hpp"
#include "entities/sparseSet.hpp"
#include "entities/entity.hpp" // Inlucde method signatures for Entity.

#include "systems/systemManager.hpp"

#include <iostream>
#include <bitset>

// Sparse set ECS implementation.

// TODO:
// Current issue, to destroy an entity, we need to iterate over the entire collection of sets
// we have, and check if the entity is a member. Instead, we want to know which sets
// an entity is a member of, to efficiently remove it from all the relevant sets.
// This also lets us query what components an entity has.

// Idea, create std::vector<signature> where size == numEntities. This way, we can instantiate the bitmask size via N_COMPONENTS, and
// have all the signatures live in the ecs root so there are no shenanigans.

namespace gxe {

template<typename ...Components>
class ecs {
// ECS templating logic, FUN :D
    static constexpr size_t N_COMPONENTS = sizeof...(Components);
    using componentSets = std::tuple<sparseSet<Components>...>;

    using signature = std::bitset<N_COMPONENTS>;
    using signatures = std::vector<signature>;

    template<typename T, typename First, typename ...Rest>
    static constexpr size_t componentIndexHelper(){
        if constexpr (std::is_same_v<T, First>){
            return 0;
        } else if constexpr (sizeof...(Rest) > 0) {
            return 1 + componentIndexHelper<T, Rest...>();
        } else {
            static_assert(sizeof(T) == 0, "Type T not found in Components");
            return 0; // Unreachable, but needed for compilation
        }
    }

    template<typename T>
    static constexpr size_t indexOf = componentIndexHelper<T, Components...>();

public:
    ecs() {
        _entities.reserve(INITIAL_SPARSE_SET_CAPACITY);
        _signatures.reserve(INITIAL_SPARSE_SET_CAPACITY);
    };
    ~ecs() = default;

    entity<Components...>& createEntity() {
        entityid id = _idManager.createEntity();
        std::cout << "Created entity " << static_cast<uint32_t>(id) << std::endl;

        if(id >= _entities.size()) {
            _entities.resize(id + INITIAL_SPARSE_SET_CAPACITY, entity<Components...>(NULL_ID, nullptr));
            _signatures.resize(id + INITIAL_SPARSE_SET_CAPACITY);
        }
        
        _entities[id] = entity<Components...>(id, this);
        return _entities[id];
    }

    // Iterate over the set bits in the entity signature, 
    // remove from the set, then set the index to null.
    // Free in the idManager.
    void destroyEntity(entityid id){

    }

    template<typename T>
    entity<Components...>& addComponent(entityid id, const T& component) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->insert(id, component);
        _signatures[id].set(indexOf<T>);
#ifdef DEBUG_SIGNATURES
        std::cout << "Added component, new bitset " << _signatures[id] << std::endl;
#endif
        return _entities[id];
    }

    template<typename T>
    entity<Components...>& removeComponent(entityid id) {
        static_assert(IsComponent<T>, "T must be a registered component type");
        
        getSet<T>()->remove(id);
        _signatures[id].reset(indexOf<T>);
#ifdef DEBUG_SIGNATURES
        std::cout << "Removed component, new bitset " << _signatures[id] << std::endl;
#endif
        return _entities[id];
    }

    template<typename T>
    const T& getComponent(entityid id) const {
        static_assert(IsComponent<T>, "T must be a registered component type");
        return const_cast<ecs*>(this)->getSet<T>()->get(id);
    }

    template<typename ...Ts>
    inline bool hasComponents(entityid id) const {
        static_assert((IsComponent<Ts> && ...), "All types must be registered components");
        auto cmp = createSignatureFromComponents<Ts...>();
        return ((cmp & _signatures[id]) == cmp);
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

        auto cmp = createSignatureFromComponents<Ts...>();
        constexpr bool enablePrefetch = sizeof...(Ts) > 2;
        for(auto& entry : smallestSet->data()) {
            if((_signatures[entry.id] & cmp) == cmp) { 
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
    
    template<typename ...Ts>
    static constexpr signature createSignatureFromComponents(){
        signature bits;
        (bits.set(indexOf<Ts>), ...);
        return bits;
    }

    std::vector<entity<Components...>> _entities;
    idManager _idManager;
    componentSets _componentSets;
    signatures _signatures;
};

} // namespace gxe