#pragma once

#include "Effect.h"
#include "raylib.h"
#include <vector>
#include <string>

struct RainDrop
{
    Vector2 pos;
    Vector2 vel;
    float length;
    float thickness;
    float alpha;
    float depth;
    bool active;
};

struct RainSplash
{
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    float maxLife;
    float alpha;
    bool active;
};

#include "LightningFlashReactive.h"
class RainEffect : public Effect, public LightningFlashReactive
{
public:
    RainEffect();
    ~RainEffect();

    void Update(float dt) override;
    void Draw() override;
    void Reset() override;

    std::string Serialize() override { return "RainData"; }
    void Deserialize(const std::string& data) override {}

    float baseWindX;
    float groundY;
    float stormIntensity;
    float gustStrength;
    float lightningFlashInfluence;
    float depthContrast;
    float burstStrength;

    void SetLightningFlashAlpha(float flash) { lightningFlashAlpha = flash; }

private:
    std::vector<RainDrop> dropsFar;
    std::vector<RainDrop> dropsMid;
    std::vector<RainDrop> dropsNear;
    std::vector<RainSplash> splashes;

    int farCount;
    int midCount;
    int nearCount;
    int maxSplashes;

    float gustTime;
    float lightningFlashAlpha;

    void ResetDrop(RainDrop& drop, bool randomY, float depthMin, float depthMax);
    void UpdateDrop(RainDrop& drop, float dt, float gustWind);
    void DrawDrop(const RainDrop& drop) const;
    void SpawnSplash(const Vector2& pos, float strength);
};