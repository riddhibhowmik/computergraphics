#include "SnowEffect.h"
#include "Easings.h"
#include "raymath.h"
#include <cstdlib>

SnowEffect::SnowEffect() : Effect("Snow") {
    xWidth = 1920.0f;
    yWidth = 1080.0f;
    speed = 100.0f;
    easingType = 0;
    directionAngle = 90.0f; // 90 degrees is straight down in Raylib's 2D coordinate system
    seed = 0;
    density = 100.0f;
    
    // TODO: Load a white ball sprite for snow here later
    // snowSprite = LoadTexture("resources/snow.png");
    
    SetSeed(seed);
    InitParticles();
}

SnowEffect::~SnowEffect() {
    // if (snowSprite.id != 0) UnloadTexture(snowSprite);
}

void SnowEffect::SetSeed(int newSeed) {
    seed = newSeed;
    srand(seed);
}

void SnowEffect::InitParticles() {
    particles.clear();
    int count = static_cast<int>(density);
    for(int i = 0; i < count; i++) {
        Particle p;
        p.position.x = static_cast<float>(rand() % static_cast<int>(xWidth));
        p.position.y = static_cast<float>(rand() % static_cast<int>(yWidth)) - yWidth; // Start above screen
        
        p.velocity.x = cos(directionAngle * DEG2RAD) * speed;
        p.velocity.y = sin(directionAngle * DEG2RAD) * speed;
        
        p.color = WHITE;
        p.size = static_cast<float>((rand() % 4) + 2); // Radius between 2 and 5
        p.lifeTime = 0.0f;
        p.maxLifeTime = 15.0f; // Snow lasts long enough to fall down
        p.isActive = true;
        
        particles.push_back(p);
    }
}

void SnowEffect::Update(float dt) {
    for(auto& p : particles) {
        if(p.isActive) {
            float t = p.lifeTime / p.maxLifeTime;
            float dt_norm = dt / p.maxLifeTime;
            
            float currentEase = Easing::Apply((EasingType)easingType, t);
            float nextEase = Easing::Apply((EasingType)easingType, t + dt_norm);
            float easeDiff = nextEase - currentEase;
            
            Vector2 totalDistance = { p.velocity.x * p.maxLifeTime, p.velocity.y * p.maxLifeTime };
            
            p.position.x += totalDistance.x * easeDiff;
            p.position.y += totalDistance.y * easeDiff;
            
            p.lifeTime += dt;
            
            // Reset particle if it goes off screen bottom or lifeTime exceeds
            if(p.position.y > yWidth || p.lifeTime > p.maxLifeTime) {
                p.position.x = static_cast<float>(rand() % static_cast<int>(xWidth));
                p.position.y = static_cast<float>(-(rand() % 100));
                p.lifeTime = 0.0f;
            }
        }
    }
}

void SnowEffect::Draw() {
    for(const auto& p : particles) {
        if(p.isActive) {
            // Placeholder: drawing circles instead of texture if sprite isn't available
            DrawCircleV(p.position, p.size, Fade(p.color, 0.8f));
        }
    }
}

std::string SnowEffect::Serialize() {
    // Basic format to save properties later
    return "Effect:Snow;Seed:" + std::to_string(seed) + ";Speed:" + std::to_string(speed) + ";Density:" + std::to_string(density);
}

void SnowEffect::Deserialize(const std::string& data) {
    // TODO: Parse the serialized string to load project effect variables
}
