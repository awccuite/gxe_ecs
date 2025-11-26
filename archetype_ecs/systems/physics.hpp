#pragma once

#include <iostream>

#include "../system.hpp"
#include "../types.hpp"

namespace gxe {

template <typename ECS>
class PhysicsSystem : public SystemCRTP<PhysicsSystem<ECS>, ECS> {
public:
    PhysicsSystem(ECS& ecs, float gravity = 9.8f) 
        : SystemCRTP<PhysicsSystem<ECS>, ECS>(60, ecs), _gravity(gravity) {}
    
    void tick(float dt) {
        std::cout << "Tick from PhysicsSystem\n";

        this->_world.template forEachWithComponents<Position, Velocity>(
            [this, dt](Position& pos, Velocity& vel) {
                vel.dy += _gravity * dt;
                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
            });
    }

private:
    float _gravity;
};

} // namespace gxe