#pragma once

#include <cstddef>
#include <bitset>
#include <tuple>

#include "components.hpp"

namespace gxe {

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

class componentSignature {
public:
    componentSignature() = default;

    // Set component as true for entity mask
    template<typename T>
    void set() { _bitset.set(componentIndex<T>, true); };

    // Set component as false for entity mask
    template<typename T>
    void reset() { _bitset.set(componentIndex<T>, false); }; // Set 0

    // Check if entity has component
    template<typename T>
    bool test() const { return _bitset.test(componentIndex<T>); }; // Check

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