#include "raylib.h"
#include "raymath.h"
#include "Particle.h"
#include <vector>

using namespace std;

const int MAX_PARTICLES = 50000;

int main()
{
    // initialize window
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "particle vfx engine");
    // SetTargetFPS(60);

    // make a 64x64 image that is white in middle and transparent on edges
    Image glowImage = GenImageGradientRadial(64, 64, 0.0f, WHITE, BLANK);
    // load it as a texture so we can draw it
    Texture2D glowTexture = LoadTextureFromImage(glowImage);
    // unload it to save memory
    UnloadImage(glowImage);

    // object pooling for particles, so make all at once and reuse as needed
    vector<Particle> particles(MAX_PARTICLES);

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].isActive = false;
    }

    // main game loop
    while (!WindowShouldClose())
    {
        // delta time to make sure movement is same on different framerates 
        float dt = GetFrameTime();

        // particle spawning vars
        int spawnRate = 35;
        int spawned = 0;
        Vector2 emitterPos = {screenWidth / 2, screenHeight - 100};

        // look for inactive particles and spawn new ones until limit reached
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (!particles[i].isActive)
            {
                particles[i].isActive = true;

                // spawn with some spread so the base is wider and more natural looking
                particles[i].position.x = emitterPos.x + GetRandomValue(-40, 40);
                particles[i].position.y = emitterPos.y + GetRandomValue(-10, 10);

                // go upwards on y, random on x
                particles[i].velocity.x = (float)GetRandomValue(-30, 30);
                particles[i].velocity.y = (float)GetRandomValue(-400, -150);
                
                // make particles random, but generally bigger
                particles[i].size = (float)GetRandomValue(80, 140);
                
                // set the lifetime to be a random number thats around 1 second
                // will make particles die at diff times and make it look more natural
                particles[i].maxLifeTime = (float)GetRandomValue(60, 150) / 100.0f;
                particles[i].lifeTime = particles[i].maxLifeTime;

                spawned++;
                if (spawned >= spawnRate)
                {
                    break;
                }
            }
        }

        // update physics
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (particles[i].isActive)
            {
                // drain lifetime 
                particles[i].lifeTime -= dt;
                
                // if lifetime is done, kill particle and skip rest of loop
                if (particles[i].lifeTime <= 0)
                {
                    particles[i].isActive = false;
                    continue;
                }

                // randomly move particle left or right
                particles[i].velocity.x += (float)GetRandomValue(-20, 20) * dt;

                // move particle, multiply by dt to make movement independent of framerate
                particles[i].position.x += particles[i].velocity.x * dt;
                particles[i].position.y += particles[i].velocity.y * dt;
            }
        }

        // draw particles
        BeginDrawing();
        ClearBackground(BLACK);

        // turn on additive blending mode so it looks more like fire
        BeginBlendMode(BLEND_ADDITIVE);

        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (particles[i].isActive)
            {
                // see how much lifetime is left in percent
                float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

                // shrink particles as they die
                float currentSize = particles[i].size * lifePercent; 

                unsigned char r, g, b, a;

                // set up a smooth fade from the yellow to the more blueish whiteish color
                if (lifePercent > 0.95f) {
                    // normalize value to be from 0 to 1
                    float t = (lifePercent - 0.95f) / 0.05f; 
                    r = (unsigned char)(255 + t * (150 - 255));
                    g = (unsigned char)(255 + t * (200 - 255));
                    b = (unsigned char)(0 + t * (255 - 0));
                }
                else {
                    // rescale percent so it transitions smoothly
                    // will have a normal yellow to orange to red fade for its remainder
                    float t = lifePercent / 0.95f;
                    r = 255;
                    g = (unsigned char)((t * t) * 255);
                    b = 0;
                }
                // fade out alpha (transparency) as it dies
                a = (unsigned char)(lifePercent * 10); 
                Color currentColor = {r, g, b, a};

                // set source to whole texture
                Rectangle source = {0, 0, (float)glowTexture.width, (float)glowTexture.height};
                // set dest to particle position and size, origin in center
                Rectangle dest = {particles[i].position.x, particles[i].position.y, currentSize, currentSize};
                // origin is in center so it can scale from middle
                Vector2 origin = {currentSize / 2, currentSize / 2}; 
                // draw particle w/ DrawTexturePro so we can scale and center it nicely
                DrawTexturePro(glowTexture, source, dest, origin, 0.0f, currentColor);
            }
        }

        // turn blending off so text is normal looking
        EndBlendMode();

        DrawText("particle emitter testing", 10, 10, 20, GREEN);
        DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}