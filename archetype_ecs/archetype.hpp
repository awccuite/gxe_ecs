#pragma once

#include "types.hpp"
#include <tuple>
#include <vector>
#include <cassert>
#include <unordered_map>

namespace gxe {

using archetypeid = entityid;
constexpr archetypeid NULL_ARCHETYPE_ID = std::numeric_limits<entityid>::max();

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

    // Add entity with components, returns archetypeID (index in this archetype)
    archetypeid addEntity(entityid id, const AComponents&... components) {
        archetypeid archId = static_cast<archetypeid>(_entityIds.size());
        
        _entityIds.push_back(id);
        _entityToArchetypeId[id] = archId;
        
        std::apply([&](auto&... vecs) {
            (vecs.push_back(components), ...);
        }, _components);
        
        return archId;
    }

    // Remove entity by entityid (performs swap-and-pop)
    void removeEntity(entityid id) {
        auto it = _entityToArchetypeId.find(id);
        if (it == _entityToArchetypeId.end()) {
            return;
        }

        archetypeid archId = it->second;
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
            
            // Update mapping for swapped entity
            _entityToArchetypeId[lastEntityId] = archId;
        }

        // Remove last elements
        _entityIds.pop_back();
        std::apply([](auto&... vecs) {
            (vecs.pop_back(), ...);
        }, _components);
        
        _entityToArchetypeId.erase(id);
    }

    // Get component for entity
    template<typename T>
    T& getComponent(entityid id) {
        static_assert((std::is_same_v<T, AComponents> || ...), "Component type not in archetype");
        
        auto it = _entityToArchetypeId.find(id);
        assert(it != _entityToArchetypeId.end() && "Entity not in archetype");
        
        archetypeid archId = it->second;
        return std::get<std::vector<T>>(_components)[archId];
    }

    template<typename T>
    const T& getComponent(entityid id) const {
        static_assert((std::is_same_v<T, AComponents> || ...), "Component type not in archetype");
        
        auto it = _entityToArchetypeId.find(id);
        assert(it != _entityToArchetypeId.end() && "Entity not in archetype");
        
        archetypeid archId = it->second;
        return std::get<std::vector<T>>(_components)[archId];
    }

    // Check if entity exists in this archetype
    bool hasEntity(entityid id) const {
        return _entityToArchetypeId.find(id) != _entityToArchetypeId.end();
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
        auto it = _entityToArchetypeId.find(id);
        assert(it != _entityToArchetypeId.end() && "Entity not in archetype");
        return it->second;
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
        _entityToArchetypeId.clear();
        std::apply([](auto&... vecs) {
            (vecs.clear(), ...);
        }, _components);
    }

private:
    std::vector<entityid> _entityIds; // archetypeID -> global entityID mapping for reverse lookup.

    // This information already lives in the ECS root. We should be able to get a pointer to our ecs root and access the efficient lookup that way.
    std::unordered_map<entityid, archetypeid> _entityToArchetypeId;  // entityID -> archetypeID mapping
    std::tuple<std::vector<AComponents>...> _components; // Component storage
};

} // namespace gxe
