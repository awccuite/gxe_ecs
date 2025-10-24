#pragma once

#include <chrono>

// A system is a function that runs repeatedly on 
// some timestep T (we can call this "ticks", = iter per second).

// Want a system to be able to run on a separate thread (eg, physics runs on a thread)

namespace gxe {

class System {
public:
    System(float tickrate) : 
        _tickrate(tickrate), 
        _tickInterval(1.0f / tickrate),
        _accumulator(0),
        _lastUpdate(std::chrono::steady_clock::now()) {};

        virtual ~System() = default;

        // called per frame
        void update() {
            auto now = std::chrono::steady_clock::now(); // Current time
            auto deltaT = now - _lastUpdate; // Time since last tick
            _lastUpdate = now;

            _accumulator += deltaT.count(); // _accumulate.

            while(_accumulator >= _tickInterval){
                tick();
                _accumulator--;
            }
        }

        float tickrate() { return _tickrate; };
        void setTickrate(float tickrate) { _tickrate = tickrate; _tickInterval = 1.0f / tickrate; };

        virtual void tick() = 0; // Tick method to be overridden in subclasses.

private:
    float _tickrate; // Ticks per second
    float _tickInterval; // Seconds per tick.
    float _accumulator; // Accumulated time since last tick
    std::chrono::steady_clock::time_point _lastUpdate; // Last tick point.
};

} // namespace gxe