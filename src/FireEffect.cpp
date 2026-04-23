#include "FireEffect.h"
#include "raymath.h"

FireEffect::FireEffect() : Effect("Fire") {
    max = 50000;
    spawnRate = 15;
    wind = 0.0f;
    spawnAccumulator = 0.0f;
    particles.resize(max);
    
    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }

    //Image glowImage = GenImageGradientRadial(64, 64, 0.0f, WHITE, BLANK);
    //glow = LoadTextureFromImage(glowImage);
    //UnloadImage(glowImage);

    fireTexture = LoadTexture("../src/smoke.png");

    for (int i = 0; i < 256; i++) {
        float t = (float)i / 255.0f;
        unsigned char r, g, b;
        if (t > 0.3f) {
            if (t > 0.9f) {
                float pct = (t - 0.9f) / 0.1f;
                r = 255;
                g = 255;
                b = (unsigned char)(200 + pct * 55);
            }
            else if (t > 0.7f) {
                float pct = (t - 0.7f) / 0.2f;
                r = 255;
                g = 255;
                b = (unsigned char)(pct * 200);
            }
            else {
                float pct = (t - 0.3f) / 0.4f;
                r = 255;
                g = (unsigned char)(100 + pct * 155);
                b = 0;
            }
        }
        else {
            float pct = t / 0.3f;
            r = g = b = (unsigned char)(pct * 165);
        }
        heatColors[i] = {r, g, b, 255};
    }
}

FireEffect::~FireEffect() {
    UnloadTexture(fireTexture);
}

void FireEffect::Update(float dt) {
    int spawned = 0;
    // center for 1080p
    Vector2 emitterPos = {(1920.0f - 480.0f) / 2.0f, 1080.0f - 250.0f}; 

    float spawns = spawnRate * 60.0f;
    spawnAccumulator += spawns * dt;
    int toSpawn = (int)spawnAccumulator;
    spawnAccumulator -= toSpawn;

    for (int i = 0; i < max; i++) {
        if (!particles[i].isActive) {
            if (spawned >= toSpawn) {
                break;
            }

            particles[i].isActive = true;
            if (GetRandomValue(1, 100) <= 15) {
                particles[i].size = (float)GetRandomValue(3, 8);
                particles[i].position.x = emitterPos.x + GetRandomValue(-40, 40);
                particles[i].position.y = emitterPos.y + GetRandomValue(-10, 20);
                particles[i].velocity.x = (float)GetRandomValue(-200, 200);
                particles[i].velocity.y = (float)GetRandomValue(-300, -150);
                particles[i].maxLifeTime = (float)GetRandomValue(50, 120) / 100.0f;
            } else {
                particles[i].size = (float)GetRandomValue(150, 250);
                particles[i].position.x = emitterPos.x + GetRandomValue(-60, 60);
                particles[i].position.y = emitterPos.y + GetRandomValue(-10, 10);
                particles[i].velocity.x = (float)GetRandomValue(-100, 100);
                particles[i].velocity.y = (float)GetRandomValue(-350, -200);
                particles[i].maxLifeTime = (float)GetRandomValue(150, 220) / 100.0f;
            }
            particles[i].lifeTime = particles[i].maxLifeTime;
            spawned++;
        }
    }

    float timeScaling = powf(0.96f, dt * 60.0f);

    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            particles[i].lifeTime -= dt;
            if (particles[i].lifeTime <= 0) {
                particles[i].isActive = false;
                continue;
            }

            float center = (1920.0f - 480.0f) / 2.0f;
            float distFromCenter = center - particles[i].position.x;

            if (particles[i].size > 10.0f) {
                float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

                if (lifePercent < 0.3f) {
                    particles[i].size += 240.0f * dt;
                    particles[i].velocity.x += wind * 4.0f * dt;
                    particles[i].velocity.y *= timeScaling;
                }
                else {
                    particles[i].velocity.x += wind * dt;
                    particles[i].velocity.y -= 50.0f * dt;
                }

                particles[i].velocity.x += distFromCenter * 1.5f * dt;
                particles[i].velocity.x += (float)GetRandomValue(-200, 200) * dt;
                particles[i].velocity.x *= timeScaling; 
            } 
            else {
                particles[i].velocity.x += distFromCenter * 1.5f * dt;
                particles[i].velocity.x += (float)GetRandomValue(-300, 300) * dt;
                particles[i].velocity.x += (wind * 1.5f) * dt; 
                particles[i].velocity.y -= 100.0f * dt;
            }
            // make the fire wobbly (flames go back and forth when rising, like a real fire)
            particles[i].velocity.x += sin(particles[i].lifeTime * 5.0f) * 40.0f * dt;

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
            float taper = pow(lifePercent, 0.5f);
            float currentSize = particles[i].size * sin(lifePercent * PI) * taper;

            Color current;

            if (particles[i].size > 10.0f) {
                int colorIndex = (int)(lifePercent * 255.0f);
                if (colorIndex < 0) {
                    colorIndex = 0;
                }
                if (colorIndex > 255) {
                    colorIndex = 255;
                }
                current = heatColors[colorIndex];
                float alpha = 1.0f;
                if (lifePercent > 0.85f) {
                    alpha = (1.0f - lifePercent) / 0.15f;
                }
                else {
                    alpha = lifePercent / 0.85f;
                }
                current.a = (unsigned char)(alpha * 255.0f);
            }
            else {
                current = WHITE;
                current.a = (unsigned char)(lifePercent * 255.0f);
            }
            
            float aspectRatio = (float)fireTexture.height / (float)fireTexture.width;
            float drawWidth = currentSize;
            float drawHeight = currentSize * aspectRatio;

            Rectangle source = {0, 0, (float)fireTexture.width, (float)fireTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, drawWidth, drawHeight};
            Vector2 origin = {drawWidth / 2.0f, drawHeight * 0.85f};

            // make the fire rotate a bit based on its position and lfietime, so it twists and turns when rising, more realistic
            float rotation = sin(particles[i].maxLifeTime * 100.0f) * 30.0f;

            DrawTexturePro(fireTexture, source, dest, origin, rotation, current);
        }
    }
    EndBlendMode();
}