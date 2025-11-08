#pragma once

#include <functional>
#include <optional>

// Systems can be created to update at some frequency T,
// or triggered manually.

// Systems should be templated over the components they access, and should maintain
// a pointer to the ECS.

// Systems should be archetype agnostic, instead utilizing components.

class SystemBase {
public:
    SystemBase(uint32_t tickrate):
    _tickrate(tickrate),
    _accumulatedTime(0.0f),
    _secsPerTick(tickrate > 0 ? 1.0f / tickrate : 0.0f) {};

    template <typename Self>
    void update(this Self&& self, float dt){ // Update method called per frame.
        // Constexpr eval should update every frame (if possible)
        if (self._tickrate == 0){
            self.tick(dt);
            return;
        }

        self._accumulatedTime += dt;

        while(self._accumulatedTime >= self._secsPerTick){
            self.tick(self._secsPerTick); // Ducktyped tick() method invocation.
            self._accumulatedTime -= self._secsPerTick;
        }
    };

    // Update method that functions regardless of dt. Requires system has per fram updates enabled.
    // template <typename Self>
    // void update(this Self&& self){
    //     static_assert(self._tickrate == 0, "Untimed system update requires tickrate == 0");
    //     self.tick(0.0f);
    // }

protected:
    const uint32_t _tickrate;
    float _accumulatedTime;
    float _secsPerTick;
};

// CRTP system interface without virtual overhead
template <typename ...C>
class System : public SystemBase {
public:
    System(uint32_t tickrate) : SystemBase(tickrate) {};
    ~System() = default;

    // Set tick to some function F.
    template<typename F>
    void tickDef(F&& lambda){ 
        _tickImpl = [lambda = std::forward<F>(lambda)](float dt) mutable {
            if constexpr (std::is_invocable_v<F, float>) {
                lambda(dt);
            } else {
                lambda();
            }
        };
    }

private:
    friend class SystemBase;
    void tick(float sysDelta) {
        if(_tickImpl){
            (*_tickImpl)(sysDelta);
        }
    }

    // Zero overhead for inherited systems,
    // only exists when needed.
    std::optional<std::function<void(float)>> _tickImpl;
    std::tuple<C...> _components;
};