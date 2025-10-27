#pragma once

#include <concepts>

// A system is a function that runs repeatedly on 
// some timestep T (we can call this "ticks", = iter per second).

// Want a system to be able to run on a separate thread (eg, physics runs on a thread)

// Idea, define multiple types of system interfaces, base, Performance critical, simple lambda
// Allow instantiation of systems to be flexible based on neccesary use cases for said systems.

namespace gxe {

class ecs; // Forward declare ecs

template<typename T> // Concept that requires a system to implement a "tick" method.
concept Tickable = requires(T t, ecs& e){
    { t.tick(e) } -> std::same_as<void>;
};
    
// CRTP style system interface using C++23 "Deducing This"
class System {
public: 
    explicit System(ecs& ecs, float tickrate = 60.0f) :
        _tickrate(tickrate),
        _tickInterval(1.0f / tickrate),
        _accumulator(0) {};

    virtual ~System() = default;

    template<typename Self>
        requires Tickable<Self>
    void update(this Self&& self, float deltaTime, ecs& ecs) {
        self._accumulator += deltaTime;
        
        while(self._accumulator >= self._tickInterval) { // While we've accumulated more delta time than the tick period.
            self.tick(ecs);  // Calls derived class method directly (no vtable)
            self._accumulator -= self._tickInterval; // Decr by tick interval.
        }
    }

    // Implement system logic within tick. Called at _tickInterval.
    virtual void tick(ecs& ecs) = 0; 

    float tickrate() const { return _tickrate; }
    float tickInterval() const { return _tickInterval; }

protected:
    float _tickrate; // Ticks per second
    float _tickInterval; // Seconds per tick.
    float _accumulator; // Accumulated time since last tick

};

} // namespace gxe