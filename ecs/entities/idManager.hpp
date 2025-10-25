#pragma once

#include "../components.hpp"

namespace gxe {

class idManager {
public:
    idManager();
    ~idManager() = default;

    entityid createEntity(); // Return an id, and remove it from availableEntities
    void destroyEntity(entityid id);

    int entityCount() { return _numEntities; };

private:
    void allocateEntities(entityid startID);
    std::vector<entityid> _availableIds; // Treat as stack for uniqueID's.

    uint32_t _numEntities;
};

}