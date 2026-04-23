#pragma once

#include "Effect.h"
#include "raylib.h"
#include <vector>
#include <string>

struct LightningSegment
{
    Vector2 start;
    Vector2 end;
    float thickness;
};

struct LightningBranch
{
    std::vector<LightningSegment> segments;
    float intensity;
};

enum class LightningPhase
{
    Warning,
    Strike,
    Fade,
    Dead
};

struct LightningBolt
{
    std::vector<LightningSegment> mainSegments;
    std::vector<LightningBranch> branches;

    // New: short-lived trail history
    std::vector<std::vector<LightningSegment>> history;

    Vector2 impactPoint;

    float timer;
    float warningDuration;
    float strikeDuration;
    float fadeDuration;

    float intensity;
    float flashStrength;

    LightningPhase phase;
    bool isActive;
};

class LightningEffect : public Effect
{
public:
    float spawnTimer;
    float nextStrikeTime;
    int maxBolts;

    float minStrikeInterval;
    float maxStrikeInterval;
    float masterFlashStrength;
    float branchIntensityMultiplier;
    float mainBoltThickness;
    float impactGlowStrength;
    int maxTrailFrames;

    LightningEffect();
    ~LightningEffect();

    void Update(float dt) override;
    void Draw() override;

    float GetGlobalFlashAlpha() const { return globalFlashAlpha; }

    std::string Serialize() override { return "LightningData"; }
    void Deserialize(const std::string& data) override {}

private:
    std::vector<LightningBolt> bolts;
    float globalFlashAlpha;

    void SpawnBolt();
    void GenerateMainBolt(LightningBolt& bolt, Vector2 start, Vector2 end, int generations, float offsetAmount);
    void SubdivideMain(std::vector<LightningSegment>& outSegments,
                       Vector2 start,
                       Vector2 end,
                       int generations,
                       float offsetAmount,
                       float thickness,
                       float verticalBias);

    void SubdivideBranch(std::vector<LightningSegment>& outSegments,
                         Vector2 start,
                         Vector2 end,
                         int generations,
                         float offsetAmount,
                         float thickness);

    float GetBoltVisualIntensity(const LightningBolt& bolt) const;
};