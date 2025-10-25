#pragma once

#include <cstdint>
#include <limits>
#include <tuple>

namespace gxe {

// Components are data, not functionality.
// Functionality lives in systems, which operate over components.

using entityid = uint32_t;
constexpr entityid NULL_ID = std::numeric_limits<entityid>::max();

enum class ComponentType {
    TRANSFORM,
    VELOCITY,
    SPRITE,
    MESH,
    GRAVITY,
    COUNT
};

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

// Forwarding for component types. Add used component types to this tuple
// for compile time resolution of SparseSets for the ECS.
using selectedComponents = std::tuple<transform, velocity, sprite, physics>;

template<typename Tuple>
struct ToSparseSets;

template<typename ...Ts>
struct ToSparseSets<std::tuple<Ts...>> {
    using type = std::tuple<sparseSet<Ts>...>;
};

using components = ToSparseSets<selectedComponents>::type;

} // Namespace gxe