#pragma once

#include <chrono>
#include <concepts>
#include <functional>

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
    
// CRTP style system class using C++23 "Deducing This"
class System {
protected: 
    // Protected base constructor to ensure only ever instantiated though inheritance
    System(float tickrate = 60.0f) : 
        _tickrate(tickrate), 
        _tickInterval(1.0f / tickrate),
        _accumulator(0),
        _tickFunc(nullptr) {};

public:
    // Lambda Constructor
    template<typename Func>
        requires std::invocable<Func, float, ecs&> // requires func is invokable and has float and ecs (match tick description.)
    System(Func&& tickFunc, float tickrate = 60.0f) : 
        _tickrate(tickrate), 
        _tickInterval(1.0f / tickrate),
        _accumulator(0),
        _tickFunc(std::forward<Func>(tickFunc)) {}; // Forward constructor for our tick function

    virtual ~System() = default;

    template<typename Self>
        requires Tickable<Self>
    void update(this Self&& self, float deltaTime, ecs& ecs) {
        self._accumulator += deltaTime; // _accumulate.

        while(self._accumulator >= self._tickInterval){
            self.tick(deltaTime, ecs);
            self._accumulator--;
        }
    }

    virtual void tick() = 0; // Tick method to be overridden in subclasses.

protected:
    float _tickrate; // Ticks per second
    float _tickInterval; // Seconds per tick.
    float _accumulator; // Accumulated time since last tick

private:
    std::function<void(float, ecs&)> _tickFunc; // std::function<returnType(...argTypes)>;
};

} // namespace gxe