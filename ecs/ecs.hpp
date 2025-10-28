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
            return 0;
        }
    }

    template<typename T>
    static constexpr size_t indexOf = componentIndexHelper<T, Components...>();

    // Type lookup by index
    template<size_t Index>
    using ComponentAt = std::tuple_element_t<Index, std::tuple<Components...>>;

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
        unsigned long long bits = _signatures[id].to_ullong();

#ifdef DEBUG_ENTITY_DESTRUCTION
        std::cout << "Destroying component at bits: ";
#endif
        
        while (bits) {
            int bitPos = __builtin_ctzll(bits);
#ifdef DEBUG_ENTITY_DESTRUCTION
            std::cout << bitPos << " ";
#endif
            removeComponentAtIndex(id, bitPos);
            bits &= (bits - 1);
        }
        
        _signatures[id].reset();
        _entities[id] = entity<Components...>(NULL_ID, nullptr);
        _idManager.destroyEntity(id);

#ifdef DEBUG_ENTITY_DESTRUCTION
        std::cout << ", destroyed entity" << std::endl;
#endif
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
    // Applies func to all components that match signatures
    // in the ecs, as such, func must be able to 
    // handle each component as an argument.
    template<typename ...Ts, typename Func>
    void forEachEntityWith(Func&& func) {
        static_assert((IsComponent<Ts> && ...), "All types must be registered components");
        
        sparseSetInterface* smallestSet = getSmallestSet<Ts...>();
        if(!smallestSet) return;

        std::bitset<N_COMPONENTS> cmp = createSignatureFromComponents<Ts...>();

        char* data = static_cast<char*>(smallestSet->rawData()); // Non-typed pointer to raw data memory.
        size_t count = smallestSet->size();
        size_t stride = smallestSet->entrySize();

        for(size_t i = 0; i < count; i++){
            entityid id = *reinterpret_cast<entityid*>(data); // Cast first 32 bits from data to id

            if((_signatures[id] & cmp) == cmp) {
                func(id, getComponent<Ts>(id)...);
            }

            data += stride;
        }
    }

    std::size_t entityCount() const {
        return _idManager.entityCount();
    }

private:
    template<typename T>
    static constexpr bool IsComponent = (std::is_same_v<T, Components> || ...);

    // Remove component by runtime index using compile-time dispatch
    template<size_t Index = 0> // Pass in index as a templated argument for our ComponentAt function.
    void removeComponentAtIndex(entityid id, size_t targetIndex) {
        if constexpr (Index < N_COMPONENTS) {
            if (Index == targetIndex) {
                getSet<ComponentAt<Index>>()->remove(id);
            } else {
                removeComponentAtIndex<Index + 1>(id, targetIndex);
            }
        }
    }

    template<typename T>
    std::size_t getSetSize() const {
        auto* set = getSet<T>();
        return set ? set->size() : 0;
    }

    template<typename First, typename ...Rest>
    sparseSetInterface* getSmallestSet() {
        if constexpr (sizeof...(Rest) == 0) {
            return getSet<First>();
        } else {
            sparseSetInterface* firstSet = getSet<First>();
            sparseSetInterface* restSmallest = getSmallestSet<Rest...>();
            
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