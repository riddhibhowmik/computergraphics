#include "FireEffect.h"
#include "raymath.h"

FireEffect::FireEffect() : Effect("Fire") {
    max = 50000;
    spawnRate = 60;
    wind = 0.0f;
    particles.resize(max);
    
    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }

    Image glowImage = GenImageGradientRadial(64, 64, 0.0f, WHITE, BLANK);
    glow = LoadTextureFromImage(glowImage);
    UnloadImage(glowImage);
}

FireEffect::~FireEffect() {
    UnloadTexture(glow);
}

void FireEffect::Update(float dt) {
    int spawned = 0;
    // center for 1080p
    Vector2 emitterPos = {1920.0f / 2.0f, 1080.0f - 250.0f}; 

    for (int i = 0; i < max; i++) {
        if (!particles[i].isActive) {
            particles[i].isActive = true;
            if (GetRandomValue(1, 100) <= 15) {
                particles[i].size = (float)GetRandomValue(3, 8);
                particles[i].position.x = emitterPos.x + GetRandomValue(-30, 30);
                particles[i].position.y = emitterPos.y + GetRandomValue(-10, 10);
                particles[i].velocity.x = (float)GetRandomValue(-100, 100);
                particles[i].velocity.y = (float)GetRandomValue(-450, -250);
                particles[i].maxLifeTime = (float)GetRandomValue(100, 250) / 100.0f;
            } else {
                particles[i].size = (float)GetRandomValue(70, 140);
                particles[i].position.x = emitterPos.x + GetRandomValue(-35, 35);
                particles[i].position.y = emitterPos.y + GetRandomValue(-10, 10);
                particles[i].velocity.x = (float)GetRandomValue(-30, 30);
                particles[i].velocity.y = (float)GetRandomValue(-300, -150);
                particles[i].maxLifeTime = (float)GetRandomValue(60, 110) / 100.0f;
            }
            particles[i].lifeTime = particles[i].maxLifeTime;
            spawned++;
            if (spawned >= spawnRate) {
                break;
            }
        }
    }

    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            particles[i].lifeTime -= dt;
            if (particles[i].lifeTime <= 0) {
                particles[i].isActive = false;
                continue;
            }

            float center = 1920.0f / 2.0f;
            float distFromCenter = center - particles[i].position.x;

            if (particles[i].size > 10.0f) {
                particles[i].velocity.x += distFromCenter * 4.0f * dt;
                particles[i].velocity.x += (float)GetRandomValue(-400, 400) * dt;
                particles[i].velocity.x += wind * dt; 
                particles[i].velocity.x *= 0.96f; 
                particles[i].velocity.y -= 150.0f * dt;
            } else {
                particles[i].velocity.x += distFromCenter * 1.5f * dt;
                particles[i].velocity.x += (float)GetRandomValue(-600, 600) * dt;
                particles[i].velocity.x += (wind * 1.5f) * dt; 
                particles[i].velocity.y -= 250.0f * dt;
            }

            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;
        }
    }
}

void FireEffect::Draw() {
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;
            float currentSize = particles[i].size * lifePercent;
            unsigned char r, g, b, a;

            if (particles[i].size > 10.0f) {
                if (lifePercent > 0.8f) {
                    float t = (lifePercent - 0.8f) / 0.2f;
                    r = 255; 
                    g = 255; 
                    b = (unsigned char)(t * 255);
                } else if (lifePercent > 0.3f) {
                    float t = (lifePercent - 0.3f) / 0.5f;
                    r = 255; 
                    g = (unsigned char)(100 + t * 155); 
                    b = 0;
                } else {
                    float t = lifePercent / 0.3f;
                    r = (unsigned char)(t * 255); 
                    g = (unsigned char)(t * 100); 
                    b = 0;
                }
                a = (unsigned char)(lifePercent * 25);
            } else {
                r = 255; 
                g = (unsigned char)(lifePercent * 255); 
                b = (unsigned char)(lifePercent * 150);
                a = (unsigned char)(lifePercent * 200);
            }

            Color current = {r, g, b, a};
            Rectangle source = {0, 0, (float)glow.width, (float)glow.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, currentSize, currentSize};
            Vector2 origin = {currentSize / 2.0f, currentSize / 2.0f};

            DrawTexturePro(glow, source, dest, origin, 0.0f, current);
        }
    }
    EndBlendMode();
}