#include "BlackHoleEffect.h"
#include "raymath.h"
#include <math.h>

BlackHoleEffect::BlackHoleEffect() : Effect("Black Hole")
{
    // we need a big swarm of particles
    max = 10000;
    spawnRate = 150;
    // we need a huge pull to make it feel like a black hole
    pull = 4000000.0f;
    spawnAccumulator = 0.0f;

    particles.resize(max);
    for (int i = 0; i < max; i++)
    {
        particles[i].isActive = false;
    }

    Image img = GenImageGradientRadial(64, 64, 0.8f, WHITE, BLANK);
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

    float uiWidth = GetScreenWidth() - 380.0f;
    float uiHeight = GetScreenHeight() - 200.0f;
    center = {uiWidth / 2.0f, uiHeight / 2.0f};

    float scale = 0.4f;

    float spawns = spawnRate * 60.0f;
    spawnAccumulator += spawns * dt;
    int toSpawn = (int)spawnAccumulator;
    spawnAccumulator -= toSpawn;

    for (int i = 0; i < max; i++)
    {
        if (!particles[i].isActive)
        {
            if (spawned >= toSpawn)
            {
                break;
            }

            particles[i].isActive = true;

            // make spawn narrower than before
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float radius = GetRandomValue(200, 500) * scale;
            particles[i].position.x = center.x + cos(angle) * radius;
            particles[i].position.y = center.y + sin(angle) * radius;

            // add a tangential velocity to make them orbit instead of falling into the center
            float tangentAngle = angle + (PI / 2.0f) + 0.3f;
            // twice as fast as before, looked too lazy
            float speed = GetRandomValue(400, 750) * scale;
            particles[i].velocity.x = cos(tangentAngle) * speed;
            particles[i].velocity.y = sin(tangentAngle) * speed;

            // give them a fairly long lifetime
            particles[i].maxLifeTime = (float)GetRandomValue(200, 400) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;
            // make particles a medioum size, allow them to grow later
            particles[i].size = (float)GetRandomValue(12, 30) * scale;

            spawned++;
        }
    }

    float timeScaling = powf(0.995, dt * 60.0f);

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

            // calculate direction to the center (singularity)
            float dx = center.x - particles[i].position.x;
            float dy = center.y - particles[i].position.y;
            float distSq = dx * dx + dy * dy;
            float dist = sqrt(distSq);

            // if they are too close, just kill them to avoid any extreme forces
            if (dist < 40.0f)
            {
                particles[i].isActive = false;
                continue;
            }

            // calculate gravity
            float force = pull / (distSq + 300.0f);

            // apply gravity 
            particles[i].velocity.x += (dx / dist) * force * dt;
            particles[i].velocity.y += (dy / dist) * force * dt;

            particles[i].velocity.x *= timeScaling;
            particles[i].velocity.y *= timeScaling;

            // move particle
            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;

            // shift color based on distancel
            unsigned char alpha = (unsigned char)(sin((particles[i].lifeTime / particles[i].maxLifeTime) * PI) * 140);

            if (dist > 130.0f) {
                // deep purple
                particles[i].color = {100, 0, 255, alpha};
            }
            else if (dist > 55.0f) {
                // hot pink
                particles[i].color = {255, 0, 200, alpha};
            }
            else {
                // bright white, bit of pink
                particles[i].color = {255, 245, 255, alpha};
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
            // make speed based on velocity so theres a motion blur kinda effect
            float speed = sqrt(particles[i].velocity.x * particles[i].velocity.x + particles[i].velocity.y * particles[i].velocity.y);
            // the faster they are, the more they shoudl stretch
            float heading = atan2(particles[i].velocity.y, particles[i].velocity.x) * RAD2DEG;

            // stretch it a lot based on how fast it is (spaghettification), some motion blur
            float stretch = 1.0f + (speed / 40.0f);
            float width = particles[i].size * stretch;
            float height = particles[i].size * (1.0f / stretch);


            Rectangle source = {0, 0, (float)blackHoleTexture.width, (float)blackHoleTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, width, height};
            Vector2 origin = {width / 2.0f, height / 2.0f};

            DrawTexturePro(blackHoleTexture, source, dest, origin, heading, particles[i].color);
        }
    }
    EndBlendMode();

    // draw the singularity on top w alpha blending
    BeginBlendMode(BLEND_ALPHA);

    // layer photon rings to make it glow, theyre white in center and more purple on the outside
    DrawCircleLines(center.x, center.y, 44.0f, {200, 0, 255, 80});       
    DrawCircleLines(center.x, center.y, 41.0f, {255, 245, 255, 200});      

    // center of black hole, just a void
    DrawCircle(center.x, center.y, 40.0f, BLACK); 

    EndBlendMode();
}