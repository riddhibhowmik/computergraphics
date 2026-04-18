#include "BlackHoleEffect.h"
#include "raymath.h"
#include <math.h>

BlackHoleEffect::BlackHoleEffect() : Effect("Black Hole")
{
    // we need a big swarm of particles
    max = 2500;
    spawnRate = 25;
    // we need a huge pull to make it feel like a black hole
    pull = 800000.0f;
    center = {1920.0f / 2.0f, 1080.0f / 2.0f};

    particles.resize(max);
    for (int i = 0; i < max; i++)
    {
        particles[i].isActive = false;
    }

    // just a glowing dot, the main thing for black hole is particle movement
    Image img = GenImageGradientRadial(16, 16, 0.0f, WHITE, BLANK);
    blackHoleTexture = LoadTextureFromImage(img);
    UnloadImage(img);
}

BlackHoleEffect::~BlackHoleEffect()
{
    UnloadTexture(blackHoleTexture);
}

void BlackHoleEffect::Update(float dt)
{
    int spawned = 0;

    for (int i = 0; i < max; i++)
    {
        if (!particles[i].isActive)
        {
            particles[i].isActive = true;

            // spawn in a big circle around the center
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float radius = GetRandomValue(500, 900);
            particles[i].position.x = center.x + cos(angle) * radius;
            particles[i].position.y = center.y + sin(angle) * radius;

            // add a tangential velocity to make them orbit instead of falling into the center
            float tangentAngle = angle + (PI / 2.0f);
            float speed = GetRandomValue(150, 400);
            particles[i].velocity.x = cos(tangentAngle) * speed;
            particles[i].velocity.y = sin(tangentAngle) * speed;

            // give them a fairly long lifetime
            particles[i].maxLifeTime = (float)GetRandomValue(400, 800) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;
            particles[i].size = (float)GetRandomValue(2, 6);

            spawned++;
            if (spawned >= spawnRate)
            {
                break;
            }
        }
    }

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

            // calculate distance to the center (singularity)
            Vector2 dir;
            dir.x = center.x - particles[i].position.x;
            dir.y = center.y - particles[i].position.y;

            float distSq = dir.x * dir.x + dir.y * dir.y;
            float dist = sqrt(distSq);

            // if they are too close, just kill them to avoid any extreme forces
            if (dist < 35.0f)
            {
                particles[i].isActive = false;
                continue;
            }

            // normalize direction
            dir.x /= dist;
            dir.y /= dist;

            // if its closer, pull is stronger, add a small constant to avoid extreme forces at close range
            float force = (pull * 100.0f) / (distSq + 1000.0f);

            // apply gravity to velocity
            particles[i].velocity.x += dir.x * force * dt;
            particles[i].velocity.y += dir.y * force * dt;

            // move particle
            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;

            // shift color based on distance
            unsigned char alpha = (unsigned char)((particles[i].lifeTime / particles[i].maxLifeTime) * 255);
            if (dist > 400.0f)
            {
                particles[i].color = {80, 0, 150, alpha}; // purple for farther particles
            }
            else if (dist > 150.0f)
            {
                particles[i].color = {0, 150, 255, alpha}; // blue for mid rangers
            }
            else
            {
                particles[i].color = {255, 255, 255, alpha}; // white for close ones
            }
        }
    }
}

void BlackHoleEffect::Draw()
{
    BeginBlendMode(BLEND_ADDITIVE);

    for (int i = 0; i < max; i++)
    {
        if (particles[i].isActive)
        {
            Rectangle source = {0, 0, (float)blackHoleTexture.width, (float)blackHoleTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, particles[i].size, particles[i].size};
            Vector2 origin = {particles[i].size / 2.0f, particles[i].size / 2.0f};

            DrawTexturePro(blackHoleTexture, source, dest, origin, 0.0f, particles[i].color);
        }
    }
    EndBlendMode();

    // draw the singularity on top w alpha blending
    BeginBlendMode(BLEND_ALPHA);

    // draw a ring to make it visible with the black background
    DrawCircleLines(center.x, center.y, 36.0f, {150, 100, 255, 200}); // purple outer rim
    DrawCircleLines(center.x, center.y, 35.0f, WHITE);                // white inner rim

    DrawCircle(center.x, center.y, 34.0f, BLACK); // black hole center

    EndBlendMode();
}