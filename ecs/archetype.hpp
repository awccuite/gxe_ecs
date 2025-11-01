#pragma once

#include <cstdint>
#include <limits>
#include <bitset>
#include <vector>

// Archetypes are templated with the components they are defined by.

namespace gxe {

using archetypeid = uint32_t;
constexpr archetypeid NULL_ARCHETYPE = std::numeric_limits<archetypeid>::max();

template<typename ...AComponents>
class archetype {
    static constexpr size_t N_COMPONENTS = sizeof...(AComponents);

public:
    // Archetypes can,
    // addEntity
    // removeEntity
    // getEntityId
    // getSwappedEntity
    // getComponent
    // forEach
    // size
    
    // Added archetype ID's are just the sizeof components[0].

private:
    std::bitset<N_COMPONENTS> _signature; // Archetypes have a signature
    std::tuple<std::vector<AComponents>...> _components; // Tuple of our components for this archetype
};

} // namespace gxe