#include "SparkEffect.h"
#include "raymath.h"

SparkEffect::SparkEffect() : Effect("Spark") {
    max = 5000;
    spawnRate = 10;
    gravity = 1200.0f;
    floorY = 1080.0f - 180.0f;
    spawnAccumulator = 0.0f;
    position = {1920.0f / 2.0f, 1080.0f - 250.0f};
    particles.resize(max);

    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }
}

SparkEffect::~SparkEffect() {}

void SparkEffect::Reset() {
    for (int i = 0; i < max; i++) {
        particles[i].isActive = false;
    }
}

void SparkEffect::Update(float dt) {
    int spawned = 0;
<<<<<<< Updated upstream
    float uiWidth = GetScreenWidth() - 380.0f;
    float uiHeight = GetScreenHeight() - 200.0f;
    Vector2 emitterPos = {uiWidth / 2.0f, uiHeight - 100.0f};

    float spawns = spawnRate * 60.0f;
    spawnAccumulator += spawns * dt;
    int toSpawn = (int)spawnAccumulator;
    spawnAccumulator -= toSpawn;

    for (int i = 0; i < max; i++) {
        if (!particles[i].isActive) {
            if (spawned >= toSpawn) {
                break;
            }

=======
    Vector2 emitterPos = position;

    for (int i = 0; i < max; i++) {
        if (!particles[i].isActive) {
            if (GetRandomValue(0, 100) > GetFadeFactor() * 100.0f) continue;
            
>>>>>>> Stashed changes
            particles[i].isActive = true;
            particles[i].position.x = emitterPos.x + GetRandomValue(-5, 5);
            particles[i].position.y = emitterPos.y + GetRandomValue(-5, 5);

            float angle = GetRandomValue(200, 340) * DEG2RAD;
            float speed = GetRandomValue(400, 1200);

            particles[i].velocity.x = cos(angle) * speed;
            particles[i].velocity.y = sin(angle) * speed;

            particles[i].maxLifeTime = (float)GetRandomValue(50, 150) / 100.0f;
            particles[i].lifeTime = particles[i].maxLifeTime;

            spawned++;
        }
    }

    const float airResistance = 0.98f;
    float timeScaling = powf(airResistance, dt * 60.0f);
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            particles[i].prev = particles[i].position;

            particles[i].velocity.y += gravity * dt;
            particles[i].velocity.x *= timeScaling;
            particles[i].velocity.y *= timeScaling;

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
    float fade = GetFadeFactor();
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < max; i++) {
        if (particles[i].isActive) {
            float lifePercent = particles[i].lifeTime / particles[i].maxLifeTime;

            unsigned char r = (unsigned char)(255 * fade);
            // square green so it fades faster
            unsigned char g = (unsigned char)(255 * (lifePercent * lifePercent) * fade);
            // cube blue so it fades even faster
            unsigned char b = (unsigned char)(150 * (lifePercent * lifePercent * lifePercent) * fade);
            unsigned char a = (unsigned char)(255 * lifePercent * fade);

            Color current = {r, g, b, a};

            // find how long particle has been alive
            float life = particles[i].maxLifeTime - particles[i].lifeTime;
            // cap stretch
            float stretchTime = fminf(1.0f / 60.0f, life);

            Vector2 tail = {particles[i].position.x - (particles[i].velocity.x * stretchTime), 
                            particles[i].position.y - (particles[i].velocity.y * stretchTime)};
            DrawLineEx(tail, particles[i].position, 2.5f, current);
        }
    }
    EndBlendMode();
}

std::string SparkEffect::Serialize() {
    return "Effect:Spark;" + SerializeBase() + "SpawnRate:" + std::to_string(spawnRate) + ";Gravity:" + std::to_string(gravity) + ";PosX:" + std::to_string(position.x) + ";PosY:" + std::to_string(position.y) + ";Active:" + std::to_string(isActive);
}

void SparkEffect::Deserialize(const std::string& data) {
    size_t pos = 0;
    std::string token;
    std::string s = data;
    while ((pos = s.find(";")) != std::string::npos) {
        token = s.substr(0, pos);
        if (!DeserializeBaseToken(token)) {
            if (token.find("SpawnRate:") == 0) spawnRate = std::stoi(token.substr(10));
            else if (token.find("Gravity:") == 0) gravity = std::stof(token.substr(8));
            else if (token.find("PosX:") == 0) position.x = std::stof(token.substr(5));
            else if (token.find("PosY:") == 0) position.y = std::stof(token.substr(5));
            else if (token.find("Active:") == 0) isActive = std::stoi(token.substr(7));
        }
        s.erase(0, pos + 1);
    }
    if (!DeserializeBaseToken(s)) {
        if (s.find("SpawnRate:") == 0) spawnRate = std::stoi(s.substr(10));
        else if (s.find("Gravity:") == 0) gravity = std::stof(s.substr(8));
        else if (s.find("PosX:") == 0) position.x = std::stof(s.substr(5));
        else if (s.find("PosY:") == 0) position.y = std::stof(s.substr(5));
        else if (s.find("Active:") == 0) isActive = std::stoi(s.substr(7));
    }
}