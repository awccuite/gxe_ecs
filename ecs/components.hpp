#pragma once

#include <cstddef>
#include <tuple>

#include <cstdint>
#include <limits>
#include <tuple>

namespace gxe {

using entityid = uint32_t;
constexpr inline entityid NULL_ID = std::numeric_limits<entityid>::max();

// DEFINE NEW COMPONENTS HERE
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

// Template logic for compile time resolution of component related classes.

template<typename T>
class sparseSet; // Forward declaration of sparseSet

template<typename Tuple>
struct ToSparseSets;

template<typename ...Ts>
struct ToSparseSets<std::tuple<Ts...>> {
    using type = std::tuple<sparseSet<Ts>...>;
};

using components = ToSparseSets<selectedComponents>::type;

// Recursive IndexOf template for setting the index of components.
template<typename T, typename Tuple, std::size_t Index = 0>
struct IndexOf;

template<typename T, typename First, typename... Rest, std::size_t Index>
struct IndexOf<T, std::tuple<First, Rest...>, Index> {
    static constexpr std::size_t value = std::is_same_v<T, First> 
        ? Index 
        : IndexOf<T, std::tuple<Rest...>, Index + 1>::value;
};

// template<typename T, std::size_t Index>
// struct IndexOf<T, std::tuple<>, Index> {
//     static_assert(sizeof(T) == 0, "Type T not found in selectedComponents tuple!");
//     static constexpr std::size_t value = Index;
// };

template<typename T>
constexpr std::size_t componentIndex = IndexOf<T, selectedComponents>::value;
inline constexpr std::size_t n_components = std::tuple_size_v<selectedComponents>;

// Index in bitset -> Component Type.
template<std::size_t Index, typename Tuple>
struct TypeAt;

template<std::size_t Index, typename...Ts>
struct TypeAt<Index, std::tuple<Ts...>> {
    using type = std::tuple_element_t<Index, std::tuple<Ts...>>;
};

template<std::size_t Index>
using ComponentAt = typename TypeAt<Index, selectedComponents>::type;

} // Namespace gxe