#pragma once
#include "Effect.h"
#include <vector>
#include "raylib.h"

using namespace std;

class FireEffect : public Effect {
    public:
        int spawnRate;
        float wind;
        int max;
        float spawnAccumulator = 0.0f;

        vector<Particle> particles;
        Texture2D glow;

        Texture2D fireTexture;
        // custom 256 color gradient array for heat, trying this out
        Color heatColors[256];

        // constuctor, destructor, and override
        FireEffect();
        ~FireEffect();

        void Update(float dt) override;
        void Draw() override;
        string Serialize() override { return "FireData"; }
        void Deserialize(const string& data) override {}
};