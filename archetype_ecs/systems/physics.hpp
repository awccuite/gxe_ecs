#pragma once

#include <iostream>

#include "../system.hpp"
#include "../types.hpp"

namespace gxe {

// The fun part now, is that we can manage system velocity inputs e.t.c.
// within the system class, allowing us to define everything in terms of m/s, px/s e.t.c should we desire.

template <typename ECS>
class PhysicsSystem : public SystemCRTP<PhysicsSystem<ECS>, ECS> {
public:
    PhysicsSystem(ECS& ecs) 
        : SystemCRTP<PhysicsSystem<ECS>, ECS>(ecs) {
            _gravity = 0.5f;
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