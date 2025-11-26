#pragma once

#include <iostream>

#include "../system.hpp"
#include "../types.hpp"

namespace gxe {

template <typename ECS>
class PhysicsSystem : public SystemCRTP<PhysicsSystem<ECS>, ECS> {
public:
    PhysicsSystem(ECS& ecs) 
        : SystemCRTP<PhysicsSystem<ECS>, ECS>(5, ecs) {
            _gravity = 9.8f;
        }
    
    void tick(float dt) {
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