#pragma once
#include "Effect.h"
#include <vector>
#include "raylib.h"

using namespace std;

struct BlackHoleParticle {
    Vector2 position;
    Vector2 velocity;
    float lifeTime;
    float maxLifeTime;
    float size;
    Color color;
    bool isActive;
};

class BlackHoleEffect : public Effect {
    public:
        int spawnRate;
        int max;
        float pull;
        Vector2 center;

        // for uncapped framerate
        float spawnAccumulator = 0.0f;

        vector<BlackHoleParticle> particles;
        Texture2D blackHoleTexture;

        BlackHoleEffect();
        ~BlackHoleEffect();

        void Update(float dt) override;
        void Draw() override;
        void Reset() override;

        string Serialize() override;
        void Deserialize(const string& data) override;
};