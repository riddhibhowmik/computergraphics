#include "SparkEffect.h"
#include "raymath.h"

SparkEffect::SparkEffect() : Effect("Spark") {
    max = 5000;
    spawnRate = 10;
    gravity = 1200.0f;
    floorY = 1080.0f - 180.0f;
    particles.resize(max);

    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }
}

SparkEffect::~SparkEffect() {}

void SparkEffect::Update(float dt) {
    int spawned = 0;
    Vector2 emitterPos = {1920.0f / 2.0f, 1080.0f - 250.0f};

    for (int i = 0; i < max; i++) {
        if (!particles[i].isActive) {
            particles[i].isActive = true;
            particles[i].position = emitterPos;
            particles[i].prev = emitterPos;

            float angle = GetRandomValue(200, 340) * DEG2RAD;
            float speed = GetRandomValue(400, 1200);

            particles[i].velocity.x = cos(angle) * speed;
            particles[i].velocity.y = sin(angle) * speed;

            particles[i].maxLifeTime = (float)GetRandomValue(50, 150) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;

            spawned++;
            if (spawned >= spawnRate) {
                break;
            }
        }
    }

    const float airResistance = 0.98f;
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            particles[i].prev = particles[i].position;

            particles[i].velocity.y += gravity * dt;
            particles[i].velocity.x *= airResistance;
            particles[i].velocity.y *= airResistance;

            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;

            if (particles[i].position.y >= floorY) {
                particles[i].position.y = floorY;
                particles[i].velocity.y *= -0.4f; 
                particles[i].velocity.x *= 0.6f;
            }

            particles[i].lifeTime -= dt;
            if (particles[i].lifeTime <= 0) {
                particles[i].isActive = false;
            }
        }
    }
}

void SparkEffect::Draw() {
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

            unsigned char r = 255;
            // square green so it fades faster
            unsigned char g = (unsigned char)(255 * (lifePercent * lifePercent));
            // cube blue so it fades even faster
            unsigned char b = (unsigned char)(150 * (lifePercent * lifePercent * lifePercent));
            unsigned char a = (unsigned char)(255 * lifePercent);

            Color current = {r, g, b, a};
            DrawLineEx(particles[i].prev, particles[i].position, 2.5f, current);
        }
    }
    EndBlendMode();
}