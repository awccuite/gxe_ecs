#pragma once

#include <bitset>

#include "components.hpp"

namespace gxe {

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

    bool empty() const { return _bitset.none(); };

    // Use intrinsics for bit scanning
    std::size_t firstSet() const {
        if(_bitset.none()) return n_components;
        return __builtin_ctzll(_bitset.to_ullong()); // clang call to find the first set bit index in the bitset.
    }
    
    std::size_t nextSet(std::size_t pos) const {
        auto shifted = _bitset >> (pos + 1);
        if(shifted.none()) return n_components;
        return (pos + 1) + __builtin_ctzll(shifted.to_ullong()); // clang call to find next bit from position that is set.
    }

private:
    std::bitset<n_components> _bitset;
};

} // namespace gxe