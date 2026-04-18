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

class NebulaEffect : public Effect {
    public:
        int spawnRate;
        int max;
        float drift;

        vector<NebulaParticle> particles;
        Texture2D cloudTexture;

        NebulaEffect();
        ~NebulaEffect();

        void Update(float dt) override;
        void Draw() override;

        string Serialize() override { return "NebulaData"; }
        void Deserialize(const string& data) override {}
};