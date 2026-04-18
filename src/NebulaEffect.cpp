#include "NebulaEffect.h"
#include "raymath.h"
#include <math.h>

NebulaEffect::NebulaEffect() : Effect("Nebula")
{
    // nebula particles are much bigger, so we need less
    max = 400;
    // keep the spawn slow and steady
    spawnRate = 2;
    drift = 15.0f;

    particles.resize(max);
    for (int i = 0; i < max; i++)
    {
        particles[i].isActive = false;
    }

    Image cloud = GenImageGradientRadial(128, 128, 0.0f, WHITE, BLANK);
    cloudTexture = LoadTextureFromImage(cloud);
    UnloadImage(cloud);
}

NebulaEffect::~NebulaEffect()
{
    UnloadTexture(cloudTexture);
}

void NebulaEffect::Update(float dt)
{
    int spawned = 0;
    Vector2 emitterPos = {1920.0f / 2.0f, 1080.0f / 2.0f - 100.0f};

    for (int i = 0; i < max; i++)
    {
        if (!particles[i].isActive)
        {
            particles[i].isActive = true;
            
            // make them spawn in a widre area to make it look mroe natural
            particles[i].position.x = emitterPos.x + GetRandomValue(-150, 150);
            particles[i].position.y = emitterPos.y + GetRandomValue(-80, 80);

            // make them pretty slow
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float speed = GetRandomValue(5, 25);
            particles[i].velocity.x = cos(angle) * speed;
            particles[i].velocity.y = sin(angle) * speed;

            // give them a pretty long lifetime
            particles[i].maxLifeTime = (float)GetRandomValue(400, 800) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;

            // let them get big
            particles[i].maxSize = (float)GetRandomValue(300, 700);
            particles[i].size = 0.0f;

            // make them rotate slowly too, overall nebula should be a big and almost lazy
            // particle, not fast like others
            particles[i].rotation = GetRandomValue(0, 360);
            particles[i].rotationSpeed = (float)GetRandomValue(-15, 15) / 10.0f;

            spawned++;
            if (spawned >= spawnRate)
            {
                break;
            }
        }
    }

    // get the global time for noise
    float time = GetTime();

    for (int i = 0; i < max; i++)
    {
        if (particles[i].isActive)
        {
            particles[i].lifeTime -= dt;
            if (particles[i].lifeTime <= 0)
            {
                particles[i].isActive = false;
                continue;
            }

            // add like pseudo noise using sine waves reliant on time and pos to make swirly patterns
            float noiseX = sin(particles[i].position.y * 0.01f + time) * drift;
            float noiseY = cos(particles[i].position.x * 0.01f + time) * drift;

            particles[i].velocity.x += noiseX * dt;
            particles[i].velocity.y += noiseY * dt;

            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;

            // grow based on lifetime
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;
            particles[i].size = particles[i].maxSize * sin(lifePercent * PI);

            particles[i].rotation += particles[i].rotationSpeed * dt;
        }
    }
}

void NebulaEffect::Draw()
{
    BeginBlendMode(BLEND_ALPHA);

    for (int i = 0; i < max; i++)
    {
        if (particles[i].isActive)
        {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

            unsigned char r, g, b;

            if (lifePercent > 0.5f)
            {
                // first half, go from cyan to a magentaish color
                float t = (lifePercent - 0.5f) * 2.0f;
                r = (unsigned char)(255 - t * 200);
                g = (unsigned char)(0 + t * 100);
                b = 255;
            }
            else
            {
                // second half, go from magenta to a deep purple
                float t = lifePercent * 2.0f;
                r = (unsigned char)(55 + t * 200);
                g = 0;
                b = (unsigned char)(100 + t * 155);
            }

            // fade in and out with a sine wave to make it look smooth
            float alphaPercent = sin(lifePercent * PI);
            // keep opacity low to make it layer better
            unsigned char a = (unsigned char)(alphaPercent * 60);

            Color current = {r, g, b, a};

            Rectangle source = {0, 0, (float)cloudTexture.width, (float)cloudTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, particles[i].size, particles[i].size};
            Vector2 origin = {particles[i].size / 2.0f, particles[i].size / 2.0f};

            DrawTexturePro(cloudTexture, source, dest, origin, particles[i].rotation, current);
        }
    }
    EndBlendMode();
}