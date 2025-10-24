#include <cstdint>
#include <iostream>

#include "idManager.hpp"
#include "components.hpp"
#include "sparseSet.hpp"

namespace gxe {

const uint32_t INITIAL_ENTITY_LIMIT = INITIAL_SPARSE_SET_CAPACITY; // Set an initial entity limit of 1024 entities.
const EntityID INITIIAL_ENTITY_ID = 0;

IdManager::IdManager(){
    allocateEntities(INITIIAL_ENTITY_ID);
}

EntityID IdManager::createEntity(){
    // Pop the back of available.
    EntityID id = _availableIds.back();
    _availableIds.pop_back();

    if(_availableIds.empty()){
        allocateEntities(id);
    }

    return id;
}

void IdManager::allocateEntities(EntityID startId){
    EntityID endId = startId + INITIAL_ENTITY_LIMIT;
    _availableIds.reserve(_availableIds.size() + INITIAL_ENTITY_LIMIT);

    std::cout << "Allocating entities " << startId + 1 << " to " << endId << "\n";

    for(EntityID i = endId; i > startId; i--){
        _availableIds.emplace_back(i);
    }
}

} // namespace gxe