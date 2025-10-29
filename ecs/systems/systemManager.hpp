#pragma once

#include <memory>
#include <vector>

#include "system.hpp"

namespace gxe {

class systemManager {

public:
    // void update();

    // template<typename System>
    // void addSystem(); // Templated addSystem method.

private:
    std::vector<std::unique_ptr<systemInterface>> _systems;

};

} // namespace gxe