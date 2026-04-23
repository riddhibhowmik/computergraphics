#pragma once
#include "Effect.h"
#include <vector>
#include "raylib.h"

using namespace std;

struct NebulaParticle {
    Vector2 position;
    Vector2 velocity;
    float lifeTime;
    float maxLifeTime;
    float size;
    float maxSize;
    float rotation;
    float rotationSpeed;
    bool isActive;
};

// adding stars just for fun, make the nebula look more like its really in space
struct Star {
    Vector2 position;
    float size;
    float baseAlpha;
    float twinkle;
    float phase;
    Color color;
};

class NebulaEffect : public Effect {
    public:
        int spawnRate;
        int max;
        float drift;
        float spawnAccumulator = 0.0f;

        vector<NebulaParticle> particles;
        Texture2D cloudTexture;

        // doing a lut for this too, worked well on fire so i think itll work here too
        Color nebulaColors[256];

        // star variables
        vector<Star> stars;
        int numStars;

        NebulaEffect();
        ~NebulaEffect();

        void Update(float dt) override;
        void Draw() override;

        string Serialize() override { return "NebulaData"; }
        void Deserialize(const string& data) override {}
};