#pragma once

#include "types.hpp"
#include <tuple>
#include <vector>
#include <cassert>


namespace gxe {

// Archetypes are templated over components
template<typename ...AComponents>
class archetype {
    static constexpr size_t N_COMPONENTS = sizeof...(AComponents);

public:
    archetype(size_t reserveSize = 128) {
        std::apply([reserveSize](auto&... vecs) {
            (vecs.reserve(reserveSize), ...);
        }, _components);
        _entityIds.reserve(reserveSize);
    }

    ~archetype() = default;

    // Set the owner ECS (must be called before using entity operations)
    void setWorld(ecs_base* world) {
        _world = world;
    }

    // Add entity with components, returns archetypeID (index in this archetype)
    archetypeid addEntity(entityid id, const AComponents&... components) {
        archetypeid archId = static_cast<archetypeid>(_entityIds.size());
        
        _entityIds.push_back(id);
        
        std::apply([&](auto&... vecs) {
            (vecs.push_back(components), ...);
        }, _components);
        
        return archId;
    }

    // Remove entity by entityid (performs swap-and-pop)
    void removeEntity(entityid id) {
        assert(_world && "Archetype owner not set");
        
        archetypeid archId = _world->getArchetypeLocalId(id);
        if (archId == NULL_ARCHETYPE_ID) {
            return;
        }

        archetypeid lastArchId = static_cast<archetypeid>(_entityIds.size() - 1);

        // Swap with last element
        if (archId != lastArchId) {
            entityid lastEntityId = _entityIds[lastArchId];
            
            // Swap entity IDs
            _entityIds[archId] = lastEntityId;
            
            // Swap components in each vector
            std::apply([archId, lastArchId](auto&... vecs) {
                ((vecs[archId] = std::move(vecs[lastArchId])), ...);
            }, _components);
            
            // Update mapping for swapped entity in the owner ECS
            _world->setArchetypeLocalId(lastEntityId, archId);
        }

        // Remove last elements
        _entityIds.pop_back();
        std::apply([](auto&... vecs) {
            (vecs.pop_back(), ...);
        }, _components);
    }

    // Get component for entity
    template<typename T>
    T& getComponent(entityid id) {
        static_assert((std::is_same_v<T, AComponents> || ...), "Component type not in archetype");
        assert(_world && "Archetype owner not set");
        
        archetypeid archId = _world->getArchetypeLocalId(id);
        assert(archId != NULL_ARCHETYPE_ID && "Entity not in archetype");
        
        return std::get<std::vector<T>>(_components)[archId];
    }

    template<typename T>
    const T& getComponent(entityid id) const {
        static_assert((std::is_same_v<T, AComponents> || ...), "Component type not in archetype");
        assert(_world && "Archetype owner not set");
        
        archetypeid archId = _world->getArchetypeLocalId(id);
        assert(archId != NULL_ARCHETYPE_ID && "Entity not in archetype");
        
        return std::get<std::vector<T>>(_components)[archId];
    }

    // Check if entity exists in this archetype
    bool hasEntity(entityid id) const {
        if (!_world) return false;
        archetypeid archId = _world->getArchetypeLocalId(id);
        return archId != NULL_ARCHETYPE_ID && archId < _entityIds.size();
    }

    template<typename C>
    static constexpr bool hasComponent(){
        return (std::is_same_v<C, AComponents> || ...);
    }

    template<typename... C>
    static constexpr bool hasComponents(){
        return (hasComponent<C>() && ...);
    }

    // Get archetypeID for entity
    archetypeid getArchetypeId(entityid id) const {
        assert(_world && "Archetype owner not set");
        archetypeid archId = _world->getArchetypeLocalId(id);
        assert(archId != NULL_ARCHETYPE_ID && "Entity not in archetype");
        return archId;
    }

    // Get entityID at archetype index
    entityid getEntityId(archetypeid archId) const {
        assert(archId < _entityIds.size() && "Invalid archetype ID");
        return _entityIds[archId];
    }

    // Iterate over all entities in archetype (all components)
    template<typename Func>
    void forEach(Func&& func) {
        size_t count = _entityIds.size();
        for (size_t i = 0; i < count; ++i) {
            entityid id = _entityIds[i];
            std::apply([&](auto&... vecs) {
                func(id, vecs[i]...);
            }, _components);
        }
    }

    // Iterate over all entities with specific components only
    template<typename... RequestedComponents, typename Func>
    void forEachWith(Func&& func) {
        static_assert(sizeof...(RequestedComponents) >= 0, "Must request at least one component");
        
        size_t count = _entityIds.size();
        for (size_t i = 0; i < count; ++i) {
            entityid id = _entityIds[i];

            if constexpr (std::is_invocable_v<Func, entityid, RequestedComponents&...>){
                func(id, std::get<std::vector<RequestedComponents>>(_components)[i]...);
            } else if constexpr (std::is_invocable_v<Func, RequestedComponents&...>){
                func(std::get<std::vector<RequestedComponents>>(_components)[i]...);
            } else {
                static_assert(std::is_invocable_v<Func, entityid, RequestedComponents&...> ||
                         std::is_invocable_v<Func, RequestedComponents&...>,
                         "Lambda must accept (entityid, Components&...) or (Components&...)");
            }
        }
    }

    size_t size() const {
        return _entityIds.size();
    }

    void clear() {
        _entityIds.clear();
        std::apply([](auto&... vecs) {
            (vecs.clear(), ...);
        }, _components);
    }

private:
    // Members
    std::vector<entityid> _entityIds; // archetypeID -> global entityID mapping for reverse lookup.
    std::tuple<std::vector<AComponents>...> _components; // Component storage

    // Reference
    ecs_base* _world = nullptr;  // Non-owning pointer to parent ECS
};

} // namespace gxe
