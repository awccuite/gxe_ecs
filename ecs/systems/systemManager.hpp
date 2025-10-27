#pragma once

#include "coreSystems.hpp"
#include "ecs/systems/system.hpp"
#include <memory>

#include <vector>

namespace gxe {

// Provide an update function that runs the systems.
// There are two types of systems, "core", and "runtime".
// Core systems are systems which we compile as part 
// of our system manager class, while runtime systems
// are those that can be defined by a user and are
// easily flexible, dynamically removed and added at runtime.
class systemManger {
public:
    void update();

private:
    coreSystems _coreSystems; // Core systems that require no overhead
    std::vector<std::unique_ptr<System>> _dynamicSystems; // Systems that are added and configured at runtime.

};

} // namespace gxe