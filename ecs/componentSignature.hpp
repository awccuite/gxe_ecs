#pragma once

#include <bitset>

#include "components.hpp"

namespace gxe {

class componentSignature {
public:
    componentSignature() = default;

    // Set component as true for entity mask
    template<typename T>
    void set() { 
        _bitset.set(componentIndex<T>(), true); 
    }

    // Set component as false for entity mask
    template<typename T>
    void reset() { 
        _bitset.reset(componentIndex<T>()); 
    }

    // Check if entity has component
    template<typename T>
    bool test() const { 
        return _bitset.test(componentIndex<T>()); 
    }

    // ComponentType enum override directly
    void set(ComponentType type) {
        _bitset.set(componentIndex(type), true);
    }

    void reset(ComponentType type) {
        _bitset.reset(componentIndex(type));
    }

    bool test(ComponentType type) const {
        return _bitset.test(componentIndex(type));
    }

    // Bulk operations
    void setAll() { _bitset.set(); }
    void resetAll() { _bitset.reset(); }

    // Check if bitsets match eachother (same components)
    bool containsAll(const componentSignature& other) const {
        return ((_bitset & other._bitset) == other._bitset);
    }

    bool intersects(const componentSignature& other) const {
        return (_bitset & other._bitset).any();
    }

    // Utility methods
    std::size_t count() const { return _bitset.count(); }
    const std::bitset<n_components>& bits() const { return _bitset; }
    bool empty() const { return _bitset.none(); }

    std::size_t firstSet() const {
        if(_bitset.none()) return n_components;
        
        #ifdef __clang__
            return __builtin_ctzll(_bitset.to_ullong());
        #else
            // Fallback for non-GCC compilers
            for(std::size_t i = 0; i < n_components; ++i) {
                if(_bitset.test(i)) return i;
            }
            return n_components;
        #endif
    }
    
    std::size_t nextSet(std::size_t pos) const {
        // Start searching from pos + 1
        for(std::size_t i = pos + 1; i < n_components; ++i) {
            if(_bitset.test(i)) return i;
        }
        return n_components;
    }

    // Optimized iteration over set bits in a signature.
    template<typename Func>
    void forEachSet(Func&& func) const {
        for(std::size_t i = firstSet(); i < n_components; i = nextSet(i)) {
            func(static_cast<ComponentType>(i));
        }
    }

private:
    std::bitset<n_components> _bitset;
};

} // namespace gxe