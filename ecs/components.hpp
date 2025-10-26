#pragma once

#include <cstddef>
#include <tuple>

#include <cstdint>
#include <limits>
#include <tuple>

namespace gxe {

using entityid = uint32_t;
constexpr inline entityid NULL_ID = std::numeric_limits<entityid>::max();

enum class ComponentType : std::size_t {
    Transform = 0,
    Velocity,
    Sprite,
    Physics,
    // Add new components here
    Count
};

// DEFINE NEW COMPONENTS HERE
struct transform {
    float x, y, z;
    float rotation;
};

struct velocity {
    float dx, dy, dz;
};

struct sprite {
    int textureId;
    float width, height;
};

struct physics {
    float mass;
};

using selectedComponents = std::tuple<transform, velocity, sprite, physics>;

// Template logic for compile time resolution of component sparse sets.
template<typename T>
class sparseSet; // Forward declaration of sparseSet

template<typename Tuple>
struct ToSparseSets;

template<typename ...Ts>
struct ToSparseSets<std::tuple<Ts...>> {
    using type = std::tuple<sparseSet<Ts>...>;
};

using components = ToSparseSets<selectedComponents>::type; // gives std::tuple<sparseSet<components>, ...>

template<typename T> 
struct ComponentID {
    // Primary template - will cause compile error if used with unregistered type
    static_assert(sizeof(T) == 0, "Component type not registered! Add ComponentID specialization.");
};

// Specializations for each component type
template<> struct ComponentID<transform> { 
    static constexpr ComponentType value = ComponentType::Transform; 
};

template<> struct ComponentID<velocity> { 
    static constexpr ComponentType value = ComponentType::Velocity; 
};

template<> struct ComponentID<sprite> { 
    static constexpr ComponentType value = ComponentType::Sprite; 
};

template<> struct ComponentID<physics> { 
    static constexpr ComponentType value = ComponentType::Physics; 
};

constexpr std::size_t componentIndex(ComponentType type) {
    return static_cast<std::size_t>(type);
}

template<typename T>
constexpr std::size_t componentIndex() {
    return componentIndex(ComponentID<T>::value);
}

// Number of components
inline constexpr std::size_t n_components = static_cast<std::size_t>(ComponentType::Count);

// Enum -> Component mapping.
template<ComponentType Type>
struct ComponentFromEnum {
    // Primary template - specialized below
};

template<> struct ComponentFromEnum<ComponentType::Transform> { using type = transform; };
template<> struct ComponentFromEnum<ComponentType::Velocity> { using type = velocity; };
template<> struct ComponentFromEnum<ComponentType::Sprite> { using type = sprite; };
template<> struct ComponentFromEnum<ComponentType::Physics> { using type = physics; };

// Alias for easier use
template<ComponentType Type>
using ComponentFromEnum_t = typename ComponentFromEnum<Type>::type;

// Runtime index to type mapping (useful for iteration over component masks)
template<std::size_t Index>
using ComponentAt = ComponentFromEnum_t<static_cast<ComponentType>(Index)>;

// Compile time checks and debug utils
template<typename T>
concept IsComponent = requires {
    { ComponentID<T>::value } -> std::convertible_to<ComponentType>;
};

// Get component name as string (useful for debugging)
constexpr const char* componentName(ComponentType type) {
    switch(type) {
        case ComponentType::Transform: return "Transform";
        case ComponentType::Velocity: return "Velocity";
        case ComponentType::Sprite: return "Sprite";
        case ComponentType::Physics: return "Physics";
        case ComponentType::Count: return "Count";
        default: return "Unknown";
    }
}

template<typename T>
constexpr const char* componentName() {
    return componentName(ComponentID<T>::value);
}

} // Namespace gxe