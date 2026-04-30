#pragma once
#include "Effect.h"
#include <vector>
#include "raylib.h"

using namespace std;

struct SparkParticle {
    Vector2 position;
    Vector2 velocity;
    Vector2 prev;
    float lifeTime;
    float maxLifeTime;
    bool isActive;
};

class SparkEffect : public Effect {
    public:
        int spawnRate;
        float gravity;
        float floorY;
        int max;
        float spawnAccumulator = 0.0f;
        Vector2 position;

        vector<SparkParticle> particles;

        SparkEffect();
        ~SparkEffect();

        void Update(float dt) override;
        void Draw() override;
        void Reset() override;
        string Serialize() override;
        void Deserialize(const string& data) override;
};