#pragma once
#include "Effect.h"
#include "raylib.h"

class SnowEffect : public Effect {
public:
    float xWidth;
    float yWidth;
    float speed;
    int easingType; // 0 = linear, etc
    float directionAngle;
    int seed;
    float density;
    
    std::vector<Particle> particles;
    Texture2D snowSprite; // For the white ball sprite

    SnowEffect();
    ~SnowEffect();

    void Update(float dt) override;
    void Draw() override;
    void Reset() override;
    void LoadResources(const std::string& projectDir) override;
    std::string Serialize() override;
    void Deserialize(const std::string& data) override;
    
    void SetSeed(int newSeed);
    void InitParticles();
};
