#include "archetype_ecs/ecs.hpp"
#include <iostream>

// Example component types
struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Health {
    int hp;
};

int main() {
    using namespace gxe;
    
    // Define archetypes
    using MovingEntity = archetype<Position, Velocity>;
    using StaticEntity = archetype<Position>;
    using Damageable = archetype<Position, Health>;
    
    // Create ECS with these archetypes
    ecs<MovingEntity, StaticEntity, Damageable> world;
    
    std::cout << "Creating entities..." << std::endl;
    
    // Create entities in different archetypes
    entityid e1 = world.createEntity<MovingEntity>(
        Position{10.0f, 20.0f},
        Velocity{1.0f, 0.5f}
    );
    std::cout << "Created moving entity: " << e1 << std::endl;
    
    entityid e2 = world.createEntity<StaticEntity>(
        Position{50.0f, 50.0f}
    );
    std::cout << "Created static entity: " << e2 << std::endl;
    
    entityid e3 = world.createEntity<Damageable>(
        Position{100.0f, 100.0f},
        Health{100}
    );
    std::cout << "Created damageable entity: " << e3 << std::endl;
    
    // Access components
    Position& pos1 = world.getComponent<MovingEntity, Position>(e1);
    std::cout << "Entity " << e1 << " position: (" << pos1.x << ", " << pos1.y << ")" << std::endl;
    
    // Iterate over all moving entities
    std::cout << "\nIterating moving entities:" << std::endl;
    world.forEach<MovingEntity>([](entityid id, Position& pos, Velocity& vel) {
        std::cout << "  Entity " << id << ": pos(" << pos.x << ", " << pos.y 
                  << ") vel(" << vel.dx << ", " << vel.dy << ")" << std::endl;
        
        // Update position
        pos.x += vel.dx;
        pos.y += vel.dy;
    });
    
    // Check updated position
    std::cout << "\nAfter update:" << std::endl;
    std::cout << "Entity " << e1 << " position: (" << pos1.x << ", " << pos1.y << ")" << std::endl;
    
    // Destroy an entity
    world.destroyEntity(e2);
    std::cout << "\nDestroyed entity " << e2 << std::endl;
    std::cout << "Entity count: " << world.entityCount() << std::endl;
    
    return 0;
}
