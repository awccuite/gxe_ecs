#pragma once

#include <cstdint>
#include <limits>
#include <tuple>

namespace gxe {

using EntityID = uint32_t;
constexpr EntityID NULL_ID = std::numeric_limits<EntityID>::max();

enum class ComponentType {
    TRANSFORM,
    VELOCITY,
    SPRITE,
    MESH,
    GRAVITY,
    COUNT
};

// Position and rotation
struct Transform {
    float x, y, z;
    float rotation;
};

struct Velocity {
    float dx, dy, dz;
};

struct Sprite {
    int textureId;
    float width, height;
};

template<typename T>
class SparseSet;

// Forwarding for component types. Add used component types to this tuple
// for compile time resolution of SparseSets for the ECS.
using SelectedComponents = std::tuple<Transform, Velocity, Sprite>;

template<typename Tuple>
struct ToSparseSets;

template<typename ...Ts>
struct ToSparseSets<std::tuple<Ts...>> {
    using type = std::tuple<SparseSet<Ts>...>;
};

using Components = ToSparseSets<SelectedComponents>::type;

} // Namespace gxe