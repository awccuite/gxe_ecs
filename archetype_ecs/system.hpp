#pragma once

#include <functional>
#include <optional>

// Systems can be created to update at some frequency T,
// or triggered manually.

class SystemBase {
public:
    SystemBase(uint32_t tickrate):
    _tickrate(tickrate),
    _accumulatedTime(0.0f),
    _secsPerTick(tickrate > 0 ? 1.0f / tickrate : 0.0f) {};

    template <typename Self>
    void update(this Self&& self, float dt){ // Update method called per frame.
        self._accumulatedTime += dt;

        while(self._accumulatedTime >= self._secsPerTick){
            self.tick(); // Ducktyped tick() method invocation.
            self._accumulatedTime -= self._secsPerTick;
        }
    };

protected:
    uint32_t _tickrate;
    float _accumulatedTime;
    float _secsPerTick;
};

// CRTP system interface without virtual overhead
class System : public SystemBase {
public:
    System(uint32_t tickrate) : SystemBase(tickrate) {};
    ~System() = default;

    // Set tick to some function F.
    template<typename F>
    void tickDef(F&& lambda){ 
        _tickImpl = std::function<void()>(std::forward<F>(lambda));
    }

private:
    friend class SystemBase;
    void tick() {
        if(_tickImpl){
            (*_tickImpl)();
        }
    }

    // Zero overhead for inherited systems,
    // only exists when needed.
    std::optional<std::function<void()>> _tickImpl;
};