#pragma once

#include <chrono>
#include <concepts>

// A system is a function that runs repeatedly on 
// some timestep T (we can call this "ticks", = iter per second).

// Want a system to be able to run on a separate thread (eg, physics runs on a thread)

// Idea, define multiple types of system interfaces, base, Performance critical, simple lambda
// Allow instantiation of systems to be flexible based on neccesary use cases for said systems.

namespace gxe {

class ecs; // Forward declare ecs

template<typename T> // Concept that requires a system to implement a "tick" method.
concept Tickable = requires(T t, float dt, ecs& e){
    { t.tick(dt, e) } -> std::same_as<void*>;
};
    
// CRTP system class using C++23 "Deducing This"
class System {
public:
    System(float tickrate) : 
        _tickrate(tickrate), 
        _tickInterval(1.0f / tickrate),
        _accumulator(0),
        _lastUpdate(std::chrono::steady_clock::now()) {};

        virtual ~System() = default;

        // called per frame
        template<typename Self>
            requires Tickable<Self>
        void update(this Self&& self, float deltaTime, ecs& ecs) {
            auto now = std::chrono::steady_clock::now(); // Current time
            auto deltaT = now - self._lastUpdate; // Time since last tick
            self._lastUpdate = now;

            self._accumulator += deltaT.count(); // _accumulate.

            while(self._accumulator >= self._tickInterval){
                self.tick(deltaTime, ecs);
                self._accumulator--;
            }
        }

        virtual void tick() = 0; // Tick method to be overridden in subclasses.

private:
    float _tickrate; // Ticks per second
    float _tickInterval; // Seconds per tick.
    float _accumulator; // Accumulated time since last tick
    std::chrono::steady_clock::time_point _lastUpdate; // Last tick point.
};

// CRTP system implementation for performance-critical processes.

// Provide super simple inline system interface intended for simple functionality
class InlineSystem : public System {

};

} // namespace gxe