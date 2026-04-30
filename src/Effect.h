#pragma once
#include <string>
#include <vector>
#include "Particle.h"

class Effect {
public:
    std::string name;
    bool isActive;
    float startTime;
    float endTime;
    int layer;
    float timelineTime;
    
    Effect(std::string name) : name(name), isActive(true), startTime(0.0f), endTime(10.0f), layer(0), timelineTime(0.0f) {}
    virtual ~Effect() = default;

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;
    virtual void Reset() = 0;
    virtual void LoadResources(const std::string& projectDir) {}
    
    // Serialization for .qfx
    virtual std::string Serialize() = 0;
    virtual void Deserialize(const std::string& data) = 0;
    
    float GetFadeFactor() const {
        float duration = endTime - startTime;
        float fadeOutTime = duration * 0.15f;
        float timeLeft = endTime - timelineTime;
        
        if (timeLeft <= 0.0f) return 0.0f;
        if (timeLeft < fadeOutTime && fadeOutTime > 0.0f) {
            return timeLeft / fadeOutTime;
        }
        return 1.0f;
    }
    
    // Helper to serialize base properties
    std::string SerializeBase() const {
        return "Start:" + std::to_string(startTime) + ";End:" + std::to_string(endTime) + ";Layer:" + std::to_string(layer) + ";";
    }
    
    // Helper to deserialize base properties, returns true if token was handled
    bool DeserializeBaseToken(const std::string& token) {
        if (token.find("Start:") == 0) { startTime = std::stof(token.substr(6)); return true; }
        if (token.find("End:") == 0) { endTime = std::stof(token.substr(4)); return true; }
        if (token.find("Layer:") == 0) { layer = std::stoi(token.substr(6)); return true; }
        return false;
    }
};
