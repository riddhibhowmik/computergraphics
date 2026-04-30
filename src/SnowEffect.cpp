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
    snowSprite.id = 0;
    
    SetSeed(seed);
    InitParticles();
}

SnowEffect::~SnowEffect() {
    if (snowSprite.id != 0) UnloadTexture(snowSprite);
}

void SnowEffect::LoadResources(const std::string& projectDir) {
    std::string pathPng = projectDir + "/sprite/snow.png";
    std::string pathJpg = projectDir + "/sprite/snow.jpg";
    if (FileExists(pathPng.c_str())) {
        if (snowSprite.id != 0) UnloadTexture(snowSprite);
        snowSprite = LoadTexture(pathPng.c_str());
    } else if (FileExists(pathJpg.c_str())) {
        if (snowSprite.id != 0) UnloadTexture(snowSprite);
        snowSprite = LoadTexture(pathJpg.c_str());
    }
}

void SnowEffect::SetSeed(int newSeed) {
    seed = newSeed;
    srand(seed);
}

void SnowEffect::Reset() {
    InitParticles();
}

void SnowEffect::InitParticles() {
    particles.clear();
    int count = static_cast<int>(density);
    for(int i = 0; i < count; i++) {
        Particle p;
        p.position.x = static_cast<float>((rand() % static_cast<int>(xWidth * 3.0f)) - xWidth);
        p.position.y = static_cast<float>(rand() % static_cast<int>(yWidth)) - yWidth; // Start above screen
        
        float speedMultiplier = ((rand() % 100) / 100.0f) * 1.0f + 0.5f;
        p.velocity.x = cos(directionAngle * DEG2RAD) * speed * speedMultiplier;
        p.velocity.y = sin(directionAngle * DEG2RAD) * speed * speedMultiplier;
        
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
                if (GetRandomValue(0, 100) > GetFadeFactor() * 100.0f) {
                    p.isActive = false;
                    continue;
                }
                
                p.position.x = static_cast<float>((rand() % static_cast<int>(xWidth * 3.0f)) - xWidth);
                p.position.y = static_cast<float>(-(rand() % 100));
                
                float speedMultiplier = ((rand() % 100) / 100.0f) * 1.0f + 0.5f;
                p.velocity.x = cos(directionAngle * DEG2RAD) * speed * speedMultiplier;
                p.velocity.y = sin(directionAngle * DEG2RAD) * speed * speedMultiplier;
                
                p.lifeTime = 0.0f;
            }
        }
    }
}

void SnowEffect::Draw() {
    float fade = GetFadeFactor();
    for(const auto& p : particles) {
        if(p.isActive) {
            if (snowSprite.id != 0) {
                // Draw texture centered on particle
                Rectangle sourceRec = {0.0f, 0.0f, (float)snowSprite.width, (float)snowSprite.height};
                Rectangle destRec = {p.position.x, p.position.y, p.size * 4.0f, p.size * 4.0f}; // Scale texture based on size
                Vector2 origin = {destRec.width / 2.0f, destRec.height / 2.0f};
                DrawTexturePro(snowSprite, sourceRec, destRec, origin, 0.0f, Fade(p.color, 0.8f * fade));
            } else {
                // Fallback: drawing circles
                DrawCircleV(p.position, p.size, Fade(p.color, 0.8f * fade));
            }
        }
    }
}

std::string SnowEffect::Serialize() {
    return "Effect:Snow;" + SerializeBase() + "Seed:" + std::to_string(seed) + ";Speed:" + std::to_string(speed) + 
           ";Density:" + std::to_string(density) + ";Angle:" + std::to_string(directionAngle) +
           ";Easing:" + std::to_string(easingType) + ";Active:" + std::to_string(isActive);
}

void SnowEffect::Deserialize(const std::string& data) {
    size_t pos = 0;
    std::string token;
    std::string s = data;
    while ((pos = s.find(";")) != std::string::npos) {
        token = s.substr(0, pos);
        if (!DeserializeBaseToken(token)) {
            if (token.find("Seed:") == 0) seed = std::stoi(token.substr(5));
            else if (token.find("Speed:") == 0) speed = std::stof(token.substr(6));
            else if (token.find("Density:") == 0) density = std::stof(token.substr(8));
            else if (token.find("Angle:") == 0) directionAngle = std::stof(token.substr(6));
            else if (token.find("Easing:") == 0) easingType = std::stoi(token.substr(7));
            else if (token.find("Active:") == 0) isActive = std::stoi(token.substr(7));
        }
        s.erase(0, pos + 1);
    }
    if (!DeserializeBaseToken(s)) {
        if (s.find("Seed:") == 0) seed = std::stoi(s.substr(5));
        else if (s.find("Speed:") == 0) speed = std::stof(s.substr(6));
        else if (s.find("Density:") == 0) density = std::stof(s.substr(8));
        else if (s.find("Angle:") == 0) directionAngle = std::stof(s.substr(6));
        else if (s.find("Easing:") == 0) easingType = std::stoi(s.substr(7));
        else if (s.find("Active:") == 0) isActive = std::stoi(s.substr(7));
    }
    
    SetSeed(seed);
    InitParticles();
}
