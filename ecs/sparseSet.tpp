#include "sparseSet.hpp"

namespace gxe {

#include <cstddef>
template<typename T>
SparseSet<T>::SparseSet() {
    sparse.resize(INITIAL_SPARSE_SET_CAPACITY, NULL_ID);
}

template<typename T>
void SparseSet<T>::insert(const EntityID id, const T& component){
    if(id >= sparse.size()){ // Resize to the id and then some.
        sparse.resize(id + INITIAL_SPARSE_SET_CAPACITY, NULL_ID);
    }

    if(sparse[id] != NULL_ID){ // Overwrite if already exists.
        dense[sparse[id]].component = component;
    }

    size_t denseIndex = static_cast<size_t>(dense.size());

    Entry e{id, component};
    dense.push_back(e);

    sparse[id] = denseIndex;
}

// Remove an entity from the sparse set. This does not free the ID though.
template<typename T>
void SparseSet<T>::remove(const EntityID id){
    if(id >= sparse.size() || sparse[id] == NULL_ID){
        return; // DNE in set
    }

    size_t denseIndex = sparse[id];
    size_t lastIndex = dense.size() - 1;

    if(denseIndex != lastIndex){
        std::swap(dense[denseIndex], dense[lastIndex]);
    }

    dense.pop_back();
    sparse[id] = NULL_ID; // No longer has representation in the sparse set.
}

// Return reference to component
template<typename T>
T& SparseSet<T>::get(const EntityID id){
    assert(has(id) && "Entity missing component");
    return dense[sparse[id]].component;
}

// Return const reference to component
template<typename T>
const T& SparseSet<T>::get(const EntityID id) const {
    assert(has(id) && "Entity missing component const");
    return dense[sparse[id]].component;
}

template<typename T>
const bool SparseSet<T>::has(const EntityID id) const {
    return id < sparse.size() && sparse[id] != NULL_ID;
}

template<typename T>
const EntityID SparseSet<T>::getEntityID(const size_t index) const {
    return dense[index].id;
}

template<typename T>
T& SparseSet<T>::getByIndex(const size_t index){
    return dense[index].component;
}

template<typename T>
size_t SparseSet<T>::size() const {
    return dense.size();
}

template<typename T>
void SparseSet<T>::clear(){
    sparse.clear();
    dense.clear();
}

} // namespace gxe