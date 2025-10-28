#pragma once

#include "../types.hpp"

#include <vector>

namespace gxe {

// Type erased sparse set interface for ECS
class sparseSetInterface {
public:
    virtual ~sparseSetInterface() = default;
    virtual void clear() = 0;
    virtual std::size_t size() const = 0;

    virtual void* rawData() = 0;
    virtual std::size_t entrySize() const = 0;
};

template<typename T>
class sparseSet : public sparseSetInterface {
public:
    sparseSet(){
        dense.reserve(INITIAL_SPARSE_SET_CAPACITY);
        sparse.reserve(INITIAL_SPARSE_SET_CAPACITY);
    };
    ~sparseSet() = default;

    void insert(const entityid, const T& component);
    void remove(const entityid);

    T& get(const entityid id); // Non const reference to the component of id
    const T& get(const entityid id) const; // Const reference to the component of id

    const bool has(const entityid id) const;
    const entityid getEntityId(const std::size_t index) const; // Get entityID for entity at index

    T& getByIndex(const std::size_t index); // Get coomponent at dense index

    std::size_t size() const override; // Return the number of components in dense

    void clear() override;

    struct entry {
        entityid id;
        T component;
    };

    const std::vector<entry>& data() const { return dense; };
    std::vector<entry>& data() { return dense; };

    void prefetch(entityid id) const {
        if(has(id)) {
            __builtin_prefetch(&dense[sparse[id]], 0, 1);
        }
    };

    void* rawData() override {
        return dense.data();
    }

    // Retun byte size of entry for set instance.
    std::size_t entrySize() const override {
        return sizeof(entry);
    }

private:
    // void rebalance(); // TODO: Rebalance the sparse set dense array.

    std::vector<entityid> sparse; // EntityID -> index in dense array. (This means we will have some wasted memory when entities do not fully saturate their components)
    std::vector<entry> dense; // Dense representation of our components. Use struct so we can extend potentially.
};

} // namespace gxe

#include "sparseSet.tpp"

