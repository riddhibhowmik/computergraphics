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

        vector<Particle> particles;
        Texture2D glow;

        // constuctor, destructor, and override
        FireEffect();
        ~FireEffect();

        void Update(float dt) override;
        void Draw() override;
        string Serialize() override { return "FireData"; }
        void Deserialize(const string& data) override {}
};