#pragma once

#include "archetype.hpp"
#include "archetype_ecs/types.hpp"
#include "idManager.hpp"
#include <tuple>
#include <vector>
#include <cassert>

namespace gxe {

// Entity record stores which archetype an entity belongs to and its position within
struct EntityRecord {
    size_t archetypeIndex; 
    
    EntityRecord() 
        : archetypeIndex(std::numeric_limits<size_t>::max()) {}
    
    EntityRecord(size_t archIdx)
        : archetypeIndex(archIdx) {}
    
    bool isValid() const {
        return archetypeIndex != std::numeric_limits<size_t>::max();
    }
};

template<typename ...Archetypes>
class ecs {
    static constexpr size_t N_ARCHETYPES = sizeof...(Archetypes);

    // Helper to get archetype index at compile time
    template<typename T, typename First, typename ...Rest>
    static constexpr size_t archetypeIndexHelper() {
        if constexpr (std::is_same_v<T, First>) {
            return 0;
        } else if constexpr (sizeof...(Rest) > 0) {
            return 1 + archetypeIndexHelper<T, Rest...>();
        } else {
            static_assert(sizeof(T) == 0, "Archetype not found");
            return 0;
        }
    }

    template<typename T>
    static constexpr size_t archetypeIndex = archetypeIndexHelper<T, Archetypes...>();

public:
    ecs() {
        _entityRecords.reserve(INITIAL_SPARSE_SET_CAPACITY);
    }
    
    ~ecs() = default;

    // Create entity in specified archetype
    template<typename Archetype, typename ...ComponentArgs>
    entityid createEntity(ComponentArgs&&... components) {
        static_assert((std::is_same_v<Archetype, Archetypes> || ...), 
                      "Archetype not registered in ECS");
        
        // Allocate global entity ID
        entityid id = _idManager.createEntity();
        
        // Ensure entity records vector is large enough
        if (id >= _entityRecords.size()) {
            _entityRecords.resize(id + INITIAL_SPARSE_SET_CAPACITY);
        }
        
        // Add entity to the archetype
        constexpr size_t archIdx = archetypeIndex<Archetype>;
        auto& arch = std::get<Archetype>(_archetypes);
        /* archetypeid archId = */ arch.addEntity(id, std::forward<ComponentArgs>(components)...);
        
        // Record the entity's location
        _entityRecords[id] = EntityRecord(archIdx);
        
        return id;
    }

    // Destroy entity from whatever archetype it's in
    void destroyEntity(entityid id) {
        assert(id < _entityRecords.size() && "Invalid entity ID");
        
        EntityRecord& record = _entityRecords[id];
        if (!record.isValid()) {
            return;
        }
        
        // Remove from archetype using runtime dispatch
        removeFromArchetype(id, record.archetypeIndex);
        
        // Clear record and free ID
        record = EntityRecord();
        _idManager.destroyEntity(id);
    }

    // Get component from entity (requires knowing which archetype)
    template<typename Archetype, typename Component>
    Component& getComponent(entityid id) {
        assert(id < _entityRecords.size() && "Invalid entity ID");
        
        EntityRecord& record = _entityRecords[id];
        assert(record.isValid() && "Entity not valid");
        assert(record.archetypeIndex == archetypeIndex<Archetype> && "Entity not in specified archetype");
        
        auto& arch = std::get<Archetype>(_archetypes);
        return arch.template getComponent<Component>(id);
    }

    // Iterate over all entities in a specific archetype
    template<typename Archetype, typename Func>
    void forEach(Func&& func) {
        auto& arch = std::get<Archetype>(_archetypes);
        arch.forEach(std::forward<Func>(func));
    }

    // Iterate over entities with only specific components
    // Archetype agnostic.
    // TODO: Evaluate if we need to pack the Components as well.
    template<typename Archetype, typename... Components, typename Func>
    void forEachWith(Func&& func) {
        auto& arch = std::get<Archetype>(_archetypes);
        arch.template forEachWith<Components...>(std::forward<Func>(func));
    }

    // For each archetype with the set of components,
    // iterate over some sub-selection of the components
    template<typename ...Components, typename Func>
    void forEachWithComponents(Func&& func){
        std::apply([&](auto&... archetypes) { // std::apply unpacks the _archetypes tuple and applies the lambda to it
            ((archetypes.template hasComponents<Components...>() ? 
                archetypes.template forEachWith<Components...>(std::forward<Func>(func)) : 
                void()), ...);
        }, _archetypes);
    }

    // Get archetype instance
    template<typename Archetype>
    Archetype& getArchetype() {
        return std::get<Archetype>(_archetypes);
    }

    template<typename Archetype>
    const Archetype& getArchetype() const {
        return std::get<Archetype>(_archetypes);
    }

    // Get entity count
    size_t entityCount() const {
        return _idManager.entityCount();
    }

    // Check if entity is valid
    bool isValid(entityid id) const {
        return id < _entityRecords.size() && _entityRecords[id].isValid();
    }

    // Get which archetype index an entity belongs to
    size_t getEntityArchetypeIndex(entityid id) const {
        assert(id < _entityRecords.size() && "Invalid entity ID");
        return _entityRecords[id].archetypeIndex;
    }

private:
    // Runtime dispatch to remove entity from archetype by index
    template<size_t Index = 0>
    void removeFromArchetype(entityid id, size_t archetypeIdx) {
        if constexpr (Index < N_ARCHETYPES) {
            if (Index == archetypeIdx) {
                using ArchetypeType = std::tuple_element_t<Index, std::tuple<Archetypes...>>;
                std::get<ArchetypeType>(_archetypes).removeEntity(id);
            } else {
                removeFromArchetype<Index + 1>(id, archetypeIdx);
            }
        }
    }

    idManager _idManager;
    std::vector<EntityRecord> _entityRecords;  // Global entity ID -> archetype location
    std::tuple<Archetypes...> _archetypes;     // All archetype instances
    // Need a way to go from component -> archetypes
};

} // namespace gxe
