#pragma once

#include <cstddef>
#include <bitset>
#include <tuple>

#include "components.hpp"

namespace gxe {

// Components are data, not functionality.
// Functionality lives in systems, which operate over components.

template<typename T>
class sparseSet;

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

class componentSignature {
public:
    componentSignature() = default;

    // TODO: Migrate to use indexOf<T>
    template<typename T>
    void set() { _bitset.set(componentIndex<T>, true); };// Set 1

    template<typename T>
    void reset() { _bitset.set(componentIndex<T>, false); }; // Set 0

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