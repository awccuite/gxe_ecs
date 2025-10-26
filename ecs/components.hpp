#pragma once

#include <cstdint>
#include <limits>
#include <tuple>

namespace gxe {

using entityid = uint32_t;
constexpr inline entityid NULL_ID = std::numeric_limits<entityid>::max();

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

using selectedComponents = std::tuple<transform, velocity, sprite, physics>;

}; // Namespace gxe