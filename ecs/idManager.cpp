#include <cstdint>
#include <iostream>

#include "idManager.hpp"
#include "components.hpp"
#include "sparseSet.hpp"

namespace gxe {

// const uint32_t INITIAL_ENTITY_LIMIT = INITIAL_SPARSE_SET_CAPACITY; // Set an initial entity limit of 1024 entities.
const uint32_t INITIAL_ENTITY_LIMIT = 32;
const entityid INITIIAL_ENTITY_ID = 0;

idManager::idManager(){
    allocateEntities(INITIIAL_ENTITY_ID);
}

entityid idManager::createEntity(){
    // Pop the back of available.
    entityid id = _availableIds.back();
    _availableIds.pop_back();

    if(_availableIds.empty()){
        allocateEntities(id);
    }

    return id;
}

void idManager::allocateEntities(entityid startId){
    entityid endId = startId + INITIAL_ENTITY_LIMIT;
    _availableIds.reserve(_availableIds.size() + INITIAL_ENTITY_LIMIT);

    std::cout << "Allocating entities " << startId + 1 << " to " << endId << "\n";

    for(entityid i = endId; i > startId; i--){
        _availableIds.emplace_back(i);
    }
}

} // namespace gxe