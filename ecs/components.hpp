#pragma once

#include <cstdint>
#include <bitset>
#include <limits>
#include <tuple>
#include <initializer_list>

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
    count // Final placeholder for num components
};

constexpr inline std::size_t n_components = static_cast<std::size_t>(ComponentType::count);

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

class componentSignature {
public:
    componentSignature() = default;

    componentSignature(std::initializer_list<ComponentType> types) {
        for (auto t : types) set(t);
    }

    void set(ComponentType t) { _bitset.set(static_cast<std::size_t>(t)); }
    void reset(ComponentType t) { _bitset.reset(static_cast<std::size_t>(t)); }
    bool test(ComponentType t) const { return _bitset.test(static_cast<std::size_t>(t)); }

    void setAll() { _bitset.set(); }
    void resetAll() { _bitset.reset(); }

    bool containsAll(const componentSignature& other) const {
        return ((_bitset & other._bitset) == other._bitset);
    }

    bool intersects(const componentSignature& other) const {
        return (_bitset & other._bitset).any();
    }

    componentSignature operator|(const componentSignature& other) const {
        componentSignature r; r._bitset = _bitset | other._bitset; return r;
    }

    componentSignature operator&(const componentSignature& other) const {
        componentSignature r; r._bitset = _bitset & other._bitset; return r;
    }

    bool operator==(const componentSignature& other) const { return _bitset == other._bitset; }
    bool operator!=(const componentSignature& other) const { return !(*this == other); }

    std::size_t count() const { return _bitset.count(); }
    const std::bitset<n_components>& bits() const { return _bitset; }

private:
    std::bitset<n_components> _bitset;
};

} // Namespace gxe