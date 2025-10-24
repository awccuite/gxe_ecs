#pragma once

#include <vector>

#include "components.hpp"

// Lets use a flag/bitset system for ID'ing which components an entity has.
// Eg, we offset by the value in our ComponentType class.

namespace gxe {

// Manages the creation and destruction of Entities (ID's). When an entity is created, we assign it an id,
// When an entity is destroyed, we remove it, and place its ID back in the collection of available ID's
class IdManager {
public:
    IdManager();
    ~IdManager() = default;

    EntityID createEntity(); // Return an id, and remove it from availableEntities
    void destroyEntity(EntityID id);
    int numEntities();

private:
    void allocateEntities(EntityID startID);
    std::vector<EntityID> _availableIds; // Treat as stack for uniqueID's.
};

}