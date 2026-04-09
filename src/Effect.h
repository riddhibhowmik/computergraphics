#pragma once
#include <string>
#include <vector>
#include "Particle.h"

class Effect {
public:
    std::string name;
    bool isActive;
    
    Effect(std::string name) : name(name), isActive(true) {}
    virtual ~Effect() = default;

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;
    
    // Serialization for .qfx
    virtual std::string Serialize() = 0;
    virtual void Deserialize(const std::string& data) = 0;
};
