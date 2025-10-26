#pragma once

#include <cstdint>
#include <vector>
#include <bitset>
#include <limits>
#include <tuple>

namespace gxe {

// Components are data, not functionality.
// Functionality lives in systems, which operate over components.

using entityid = uint32_t;
constexpr inline entityid NULL_ID = std::numeric_limits<entityid>::max();

enum class ComponentType {
    transform,
    velocity,
    sprite,
    physics,
    count
};

constexpr std::size_t n_components = static_cast<std::size_t>(ComponentType::count);

// Position and rotation
struct transform {
    float x, y, z;
    float rotation;
};

struct velocity {
    float dx, dy, dz;
};

struct sprite {
    int textureId;
    float width, height;
};

struct physics {
    float mass;
};

template<typename T>
class sparseSet;

using selectedComponents = std::tuple<transform, velocity, sprite, physics>;

template<typename Tuple>
struct ToSparseSets;

template<typename ...Ts>
struct ToSparseSets<std::tuple<Ts...>> {
    using type = std::tuple<sparseSet<Ts>...>;
};

using components = ToSparseSets<selectedComponents>::type;

struct componentSignature {
    std::bitset<n_components> _bitset;
};

} // Namespace gxe