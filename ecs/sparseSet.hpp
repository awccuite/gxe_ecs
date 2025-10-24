#pragma once

#include "idManager.hpp"
#include <cstddef>

namespace gxe {

constexpr size_t INITIAL_SPARSE_SET_CAPACITY = 1024;

// Type erased sparse set interface for ECS
class SparseSetInterface {
public:
    virtual ~SparseSetInterface() = default;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
};

template<typename T>
class SparseSet : public SparseSetInterface {
public:
    SparseSet();
    ~SparseSet() = default;

    void insert(const EntityID, const T& component);
    void remove(const EntityID);

    T& get(const EntityID id); // Non const reference to the component of id
    const T& get(const EntityID id) const; // Const reference to the component of id

    const bool has(const EntityID id) const;
    const EntityID getEntityID(const size_t index) const; // Get entityID for entity at index

    T& getByIndex(const size_t index); // Get coomponent at dense index

    size_t size() const override; // Return the number of components in dense

    void clear() override;

    struct Entry {
        EntityID id;
        T component;
    };

    std::vector<Entry>& data(){
        return dense;
    }

private:
    std::vector<EntityID> sparse; // EntityID -> index in dense array. (This means we will have some wasted memory when entities do not fully saturate their components)
    std::vector<Entry> dense; // Dense representation of our components. Use struct so we can extend potentially.

};

} // namespace gxe

#include "sparseSet.tpp"

