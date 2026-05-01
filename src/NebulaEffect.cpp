#include "NebulaEffect.h"
#include "raymath.h"
#include <math.h>

NebulaEffect::NebulaEffect() : Effect("Nebula")
{
    // nebula particles are much bigger, so we need less
    max = 3000;
    // keep the spawn slow and steady
    spawnRate = 2;
    drift = 15.0f;
    position = {1920.0f / 2.0f, 1080.0f / 2.0f - 100.0f};

    particles.resize(max);
    for (int i = 0; i < max; i++)
    {
        particles[i].isActive = false;
    }

    numStars = 400;
    stars.resize(numStars);
    for (int i = 0; i < numStars; i++) {

        //  make sure they spawn everywhere for that space feel 
        stars[i].position.x = (float)GetRandomValue(-500, 2500);
        stars[i].position.y = (float)GetRandomValue(-500, 1500);

        // sizes should be roughly the same, a bit of variation to make it look more natural visually
        stars[i].size = (float)GetRandomValue(5, 15) / 10.0f;
        // low opacity
        stars[i].baseAlpha = (float)GetRandomValue(30, 120);

        // make the twinkling of stars and the phasing random so they all look different
        stars[i].twinkle = (float)GetRandomValue(10, 40) / 10.0f;
        stars[i].phase = (float)GetRandomValue(0, 360) * DEG2RAD;

        int colorType = GetRandomValue(1, 100);
        if (colorType > 95) {
            // rare warm orange star
            stars[i].color = {255, 200, 150, 255};
        }
        else if (colorType > 80) {
            // less rare, cool blue star
            stars[i].color = {180, 220, 255, 255};
        }
        else {
            // mostly white stars
            stars[i].color = {255, 255, 255, 255};
        }
    }

    Image cloud = GenImageGradientRadial(256, 256, 0.5f, WHITE, BLANK);
    cloudTexture = LoadTextureFromImage(cloud);
    UnloadImage(cloud);

    // do the lut math
    for (int i = 0; i < 256; i++)
    {
        float t = (float)i / 255.0f;
        unsigned char r, g, b;

        if (t > 0.6f)
        {
            // first bit, the core of it, bright cyan and a deeper blue
            float t2 = (t - 0.6f) / 0.4f;
            r = (unsigned char)(t2 * 100);
            g = (unsigned char)(150 + t2 * 105);
            b = 255;
        }
        else if (t > 0.2f)
        {
            // middle bit, body of nebula, basiclaly just pink and red
            float t2 = (t - 0.2f) / 0.4f;
            r = (unsigned char)(200 + t2 * 55);
            g = (unsigned char)(t2 * 50);
            b = (unsigned char)(100 + t2 * 100);
        }
        else
        {
            // last bit, go to a deep cosmic red
            float t2 = t / 0.2f;
            r = (unsigned char)(50 + t2 * 150);
            g = (unsigned char)(t2 * 30);
            b = (unsigned char)(t2 * 50);
        }
        nebulaColors[i] = {r, g, b, 255};
    }
}

NebulaEffect::~NebulaEffect()
{
    UnloadTexture(cloudTexture);
}

void NebulaEffect::Reset() {
    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }
}

void NebulaEffect::Update(float dt)
{
    int spawned = 0;
    Vector2 emitterPos = position;

    for (int i = 0; i < max; i++)
    {
        if (!particles[i].isActive)
        {
            if (GetRandomValue(0, 100) > GetFadeFactor() * 100.0f) continue;
            
            particles[i].isActive = true;

            // make them spawn in a widre area to make it look mroe natural
            float spawnRadius = 0.0f;
            bool isDust = GetRandomValue(1, 100) > 85;

            // Default scale since it was missing
            float scale = 0.35f;

            if (isDust) {
                // if its dust, spawn it further out, so theyll never be in the core and swallow light
                spawnRadius = (float)GetRandomValue(80, 250) * scale;
                particles[i].maxSize = -(float)GetRandomValue(600, 1200) * scale;
            }
            else {
                // if its gas, spawn it in the center, no holes in the middle of nebula
                if (GetRandomValue(1, 100) > 30) {
                    // most should spawn in the inner area
                    spawnRadius = (float)GetRandomValue(0, 70) * scale;
                }
                else {
                    // still have a bit of wisps outwards
                    spawnRadius = (float)GetRandomValue(70, 250) * scale;
                }
                particles[i].maxSize = (float)GetRandomValue(200, 800) * scale;
            }

            float spawnAngle = GetRandomValue(0, 360) * DEG2RAD;
            particles[i].position.x = emitterPos.x + cos(spawnAngle) * spawnRadius;
            // more oval shaped spawn
            particles[i].position.y = emitterPos.y + sin(spawnAngle) * (spawnRadius * 0.7f);

            // make them pretty slow
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float speed = GetRandomValue(5, 25);
            particles[i].velocity.x = cos(angle) * speed;
            particles[i].velocity.y = sin(angle) * speed;

            // give them a pretty long lifetime
            particles[i].maxLifeTime = (float)GetRandomValue(400, 800) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;

            if (GetRandomValue(1, 100) > 85)
            {
                // if its negative, itll be a dark dust, so itll block light and look like a hole
                particles[i].maxSize = -(float)GetRandomValue(600, 1200) * scale;
            }
            else
            {
                particles[i].maxSize = (float)GetRandomValue(200, 800) * scale;
            }

            particles[i].size = 0.0f;

            // make them rotate slowly too, overall nebula should be a big and almost lazy
            // particle, not fast like others
            particles[i].rotation = GetRandomValue(0, 360);
            // slow rotation, they should be chill and lazy
            particles[i].rotationSpeed = (float)GetRandomValue(-5, 5) / 10.0f;

            spawned++;
        }
    }

    // get the global time for noise
    float time = (float)GetTime();

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

            // add like pseudo noise using sine waves reliant on time and pos to make swirly patterns, make sure the path
            // isn't linear and more spacey
            float noiseX = sin(particles[i].position.y * 0.01f + time) * drift;
            noiseX += sin(particles[i].position.y * 0.04f - time * 1.8f) * (drift * 0.6f);
            float noiseY = cos(particles[i].position.x * 0.01f + time) * drift;
            noiseY += cos(particles[i].position.x * 0.04f - time * 1.8f) * (drift * 0.6f);

            particles[i].velocity.x += noiseX * dt;
            particles[i].velocity.y += noiseY * dt;

            // add a spin, use these to calculate tangent vector
            float dx = particles[i].position.x - emitterPos.x;
            float dy = particles[i].position.y - emitterPos.y;
            float distance = sqrt(dx * dx + dy * dy);

            // make sure cant divide by 0
            if (distance > 0.1f) {
                // angle to rotate the frame at
                float angular = (15.0f / (distance + 50.0f)) * dt;

                float dx2 = dx * cos(angular) - dy * sin(angular);
                float dy2 = dx * sin(angular) + dy * cos(angular);

                particles[i].position.x = emitterPos.x + dx2;
                particles[i].position.y = emitterPos.y + dy2;
            }

            // apply noise for swirly patterns and stabilization
            particles[i].velocity.x += noiseX * dt;
            particles[i].velocity.y += noiseY * dt;

            // further clouds (larger ones) should move a bit slowerr for a depth effect and it looks cooler
            float speedMult = 1.0f - (fabs(particles[i].maxSize) / 2000.0f);
            particles[i].position.x += (particles[i].velocity.x * speedMult) * dt;
            particles[i].position.y += (particles[i].velocity.y * speedMult) * dt;

            // grow based on lifetime
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;
            particles[i].size = particles[i].maxSize * sin(lifePercent * PI);

            particles[i].rotation += particles[i].rotationSpeed * dt;
        }
    }
}

void NebulaEffect::Draw()
{
    float fade = GetFadeFactor();
    
    // get the time to make stars twinkle every so often, use global so its consistent
    float time = (float)GetTime();

    // draw stars with additive blending so they glow too, draw them first so they can be behind the nebula
    // so that that way it looks like the nebula is in space
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < numStars; i++) {
        // sine wave math for twinkling
        float alphaOffset = sin(time * stars[i].twinkle + stars[i].phase) * 40.0f;
        int currentAlpha = (int)stars[i].baseAlpha + (int)alphaOffset;

        // clamp so it doesnt become a weird color thats out of bounds
        currentAlpha = Clamp(currentAlpha, 0, 255);
        Color draw = stars[i].color;
        draw.a = (unsigned char)currentAlpha;

        // use draw circle v for performance, they dont need crazy textures so we can get away with just circles
        DrawCircleV(stars[i].position, stars[i].size, draw);
    }

    EndBlendMode();
    // additive blending for the glowing gas particles
    BeginBlendMode(BLEND_ADDITIVE);

    for (int i = 0; i < max; i++) {
        if (particles[i].isActive && particles[i].maxSize > 0) {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

            int colorIndex = (int)(lifePercent * 255);
            colorIndex = Clamp(colorIndex, 0, 255);
            Color current = nebulaColors[colorIndex];

            float alphaMult = pow(sin(lifePercent * PI), 1.5f); 
            current.a = (unsigned char)(60.0f * alphaMult);

            // fade in and out with a sine wave to make it look smooth
            float alphaPercent = sin(lifePercent * PI);
            // keep opacity low to make it layer better
            unsigned char a = (unsigned char)(alphaPercent * 60);
            
            current.r = (unsigned char)(current.r * fade);
            current.g = (unsigned char)(current.g * fade);
            current.b = (unsigned char)(current.b * fade);
            current.a = (unsigned char)(a * fade);

            float actualSize = fabs(particles[i].size);
            float stretch = 1.2f + (sin(lifePercent * PI * 0.5f) * 0.3f);
            float drawWidth = actualSize * stretch;
            float drawHeight = actualSize * (1.0f / stretch);

            Rectangle source = {0, 0, (float)cloudTexture.width, (float)cloudTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, drawWidth, drawHeight};
            Vector2 origin = {cloudTexture.width / 2.0f, cloudTexture.height / 2.0f};

            DrawTexturePro(cloudTexture, source, dest, origin, particles[i].rotation, current);
        }
    }
    EndBlendMode();

    // alpha blending for darker dusty particles, so they can block light
    BeginBlendMode(BLEND_ALPHA);
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive && particles[i].maxSize < 0) {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

            Color current = {5, 5, 10, 255};

            float alphaMult = pow(sin(lifePercent * PI), 1.5f);
            current.a = (unsigned char)(35.0f * alphaMult * fade);

            float actualSize = fabs(particles[i].size);
            float stretch = 1.2f + (sin(lifePercent * PI * 0.5f) * 0.3f);
            float drawWidth = actualSize * stretch;
            float drawHeight = actualSize / (1.0f / stretch);

            Rectangle source = {0, 0, (float)cloudTexture.width, (float)cloudTexture.height};
            Rectangle dest = {particles[i].position.x, particles[i].position.y, drawWidth, drawHeight};
            Vector2 origin = {cloudTexture.width / 2.0f, cloudTexture.height / 2.0f};

            DrawTexturePro(cloudTexture, source, dest, origin, particles[i].rotation, current);
        }
    }
    EndBlendMode();
}

std::string NebulaEffect::Serialize() {
    return "Effect:Nebula;" + SerializeBase() + "SpawnRate:" + std::to_string(spawnRate) + ";Drift:" + std::to_string(drift) + ";PosX:" + std::to_string(position.x) + ";PosY:" + std::to_string(position.y) + ";Active:" + std::to_string(isActive);
}

void NebulaEffect::Deserialize(const std::string& data) {
    size_t pos = 0;
    std::string token;
    std::string s = data;
    while ((pos = s.find(";")) != std::string::npos) {
        token = s.substr(0, pos);
        if (!DeserializeBaseToken(token)) {
            if (token.find("SpawnRate:") == 0) spawnRate = std::stoi(token.substr(10));
            else if (token.find("Drift:") == 0) drift = std::stof(token.substr(6));
            else if (token.find("PosX:") == 0) position.x = std::stof(token.substr(5));
            else if (token.find("PosY:") == 0) position.y = std::stof(token.substr(5));
            else if (token.find("Active:") == 0) isActive = std::stoi(token.substr(7));
        }
        s.erase(0, pos + 1);
    }
    if (!DeserializeBaseToken(s)) {
        if (s.find("SpawnRate:") == 0) spawnRate = std::stoi(s.substr(10));
        else if (s.find("Drift:") == 0) drift = std::stof(s.substr(6));
        else if (s.find("PosX:") == 0) position.x = std::stof(s.substr(5));
        else if (s.find("PosY:") == 0) position.y = std::stof(s.substr(5));
        else if (s.find("Active:") == 0) isActive = std::stoi(s.substr(7));
    }
}