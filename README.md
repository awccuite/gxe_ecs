# Archetype-Based ECS

An archetype-based Entity Component System implementation.

## Architecture

### Core Components

1. **`archetype.hpp`** - Template class for managing entities with a specific set of components
   - Stores components in parallel arrays (SoA - Structure of Arrays)
   - Maintains bidirectional mapping between entity IDs and archetype IDs
   - Uses swap-and-pop for efficient entity removal

2. **`ecs.hpp`** - Main ECS coordinator
   - Manages global entity ID allocation
   - Maintains `EntityRecord` for each entity (tracks which archetype and position)
   - Provides type-safe entity creation and destruction
   - Dispatches operations to appropriate archetypes

## Usage

### 1. Define Component Types
```cpp
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
```

### 2. Define Archetypes
```cpp
using MovingEntity = gxe::archetype<Position, Velocity>;
using StaticEntity = gxe::archetype<Position>;
using Damageable = gxe::archetype<Position, Health>;
```

### 3. Create ECS
```cpp
gxe::ecs<MovingEntity, StaticEntity, Damageable> world;
```

### 4. Create Entities
```cpp
// Create entity in MovingEntity archetype
entityid id = world.createEntity<MovingEntity>(
    Position{10.0f, 20.0f},
    Velocity{1.0f, 0.5f}
);
```

### 5. Access Components
```cpp
Position& pos = world.getComponent<MovingEntity, Position>(id);
pos.x += 5.0f;
```

### 6. Iterate Over Archetype
```cpp
world.forEach<MovingEntity>([](entityid id, Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});
```

### 7. Destroy Entity
```cpp
world.destroyEntity(id);
```

### 8. Building
Built with CMake and Clang (Requires C++ 20)
