#include "LightningEffect.h"
#include "raymath.h"
#include <cmath>

LightningEffect::LightningEffect() : Effect("Lightning")
{
    spawnTimer = 0.0f;
    nextStrikeTime = 1.8f;
    maxBolts = 6;
    globalFlashAlpha = 0.0f;

    minStrikeInterval = 1.2f;
    maxStrikeInterval = 3.6f;
    masterFlashStrength = 1.0f;
    branchIntensityMultiplier = 1.0f;
    mainBoltThickness = 4.8f;
    impactGlowStrength = 1.0f;
    maxTrailFrames = 3;

    bolts.resize(maxBolts);
    for (int i = 0; i < maxBolts; i++)
        bolts[i].isActive = false;
}

LightningEffect::~LightningEffect() {}

float LightningEffect::GetBoltVisualIntensity(const LightningBolt& bolt) const
{
    switch (bolt.phase)
    {
        case LightningPhase::Warning:
            return 0.18f * bolt.intensity;
        case LightningPhase::Strike:
            return 1.00f * bolt.intensity;
        case LightningPhase::Fade:
        {
            float t = 1.0f - (bolt.timer / bolt.fadeDuration);
            return fmaxf(0.0f, t) * 0.70f * bolt.intensity;
        }
        default:
            return 0.0f;
    }
}

void LightningEffect::Update(float dt)
{
    spawnTimer += dt;
    globalFlashAlpha = 0.0f;

    if (spawnTimer >= nextStrikeTime)
    {
        spawnTimer = 0.0f;

        float intervalRange = fmaxf(0.1f, maxStrikeInterval - minStrikeInterval);
        nextStrikeTime = minStrikeInterval + ((float)GetRandomValue(0, 1000) / 1000.0f) * intervalRange;

        SpawnBolt();
    }

    for (int i = 0; i < maxBolts; i++)
    {
        LightningBolt& bolt = bolts[i];
        if (!bolt.isActive) continue;

        bolt.timer += dt;

        if (bolt.phase == LightningPhase::Warning)
        {
            globalFlashAlpha = fmaxf(globalFlashAlpha, 18.0f * bolt.flashStrength * masterFlashStrength);

            if (bolt.timer >= bolt.warningDuration)
            {
                bolt.phase = LightningPhase::Strike;
                bolt.timer = 0.0f;

                bolt.history.clear();
                bolt.history.push_back(bolt.mainSegments);
            }
        }
        else if (bolt.phase == LightningPhase::Strike)
        {
            float pulse = 0.85f + 0.15f * sinf((float)GetTime() * 90.0f);
            globalFlashAlpha = fmaxf(globalFlashAlpha, 75.0f * bolt.flashStrength * masterFlashStrength * pulse);

            // Keep a few ghost copies for trailing afterglow
            if ((int)bolt.history.size() < maxTrailFrames)
            {
                bolt.history.push_back(bolt.mainSegments);
            }

            if (bolt.timer >= bolt.strikeDuration)
            {
                bolt.phase = LightningPhase::Fade;
                bolt.timer = 0.0f;
            }
        }
        else if (bolt.phase == LightningPhase::Fade)
        {
            float t = 1.0f - (bolt.timer / bolt.fadeDuration);
            globalFlashAlpha = fmaxf(globalFlashAlpha, 28.0f * bolt.flashStrength * masterFlashStrength * fmaxf(0.0f, t));

            if (bolt.timer >= bolt.fadeDuration)
            {
                bolt.phase = LightningPhase::Dead;
                bolt.isActive = false;
                bolt.history.clear();
            }
        }
    }
}

void LightningEffect::Draw()
{
    if (globalFlashAlpha > 0.0f)
    {
        DrawRectangle(0, 0, 1920, 1080,
            Color{210, 220, 255, (unsigned char)fminf(globalFlashAlpha, 120.0f)});
    }

    BeginBlendMode(BLEND_ADDITIVE);

    for (int i = 0; i < maxBolts; i++)
    {
        const LightningBolt& bolt = bolts[i];
        if (!bolt.isActive) continue;

        float visual = GetBoltVisualIntensity(bolt);
        float flicker = 0.88f + 0.12f * sinf((float)GetTime() * 120.0f + i * 2.7f);
        float brightness = visual * flicker;

        // Draw faint trail history first
        if (!bolt.history.empty())
        {
            for (size_t h = 0; h < bolt.history.size(); h++)
            {
                float trailT = (float)(h + 1) / (float)bolt.history.size();
                float trailAlpha = 18.0f + 18.0f * trailT;

                for (const LightningSegment& seg : bolt.history[h])
                {
                    DrawLineEx(
                        seg.start,
                        seg.end,
                        seg.thickness * 0.95f,
                        Color{170, 195, 255, (unsigned char)(trailAlpha * brightness)}
                    );
                }
            }
        }

        for (const LightningSegment& seg : bolt.mainSegments)
        {
            float outerThickness = seg.thickness * 3.4f;
            float midThickness = seg.thickness * 2.0f;
            float innerThickness = seg.thickness;

            Color outerGlow = { 90, 150, 255, (unsigned char)(60 * brightness) };
            Color midGlow   = { 140, 190, 255, (unsigned char)(100 * brightness) };
            Color core      = { 240, 245, 255, (unsigned char)(255 * brightness) };

            DrawLineEx(seg.start, seg.end, outerThickness, outerGlow);
            DrawLineEx(seg.start, seg.end, midThickness, midGlow);
            DrawLineEx(seg.start, seg.end, innerThickness, core);
        }

        for (const LightningBranch& branch : bolt.branches)
        {
            float branchBrightness = brightness * branch.intensity * branchIntensityMultiplier;

            for (const LightningSegment& seg : branch.segments)
            {
                float glowThickness = seg.thickness * 2.2f;
                Color branchGlow = { 100, 160, 255, (unsigned char)(45 * branchBrightness) };
                Color branchCore = { 220, 235, 255, (unsigned char)(180 * branchBrightness) };

                DrawLineEx(seg.start, seg.end, glowThickness, branchGlow);
                DrawLineEx(seg.start, seg.end, seg.thickness, branchCore);
            }
        }

        float impactRadius = (30.0f + 55.0f * brightness) * impactGlowStrength;
        DrawCircleV(
            bolt.impactPoint,
            impactRadius,
            Color{180, 210, 255, (unsigned char)(32 * brightness * impactGlowStrength)}
        );
        DrawCircleV(
            bolt.impactPoint,
            impactRadius * 0.45f,
            Color{240, 245, 255, (unsigned char)(95 * brightness * impactGlowStrength)}
        );
    }

    EndBlendMode();
}

void LightningEffect::SpawnBolt()
{
    for (int i = 0; i < maxBolts; i++)
    {
        if (bolts[i].isActive) continue;

        LightningBolt& bolt = bolts[i];
        bolt.mainSegments.clear();
        bolt.branches.clear();
        bolt.history.clear();
        bolt.isActive = true;
        bolt.phase = LightningPhase::Warning;
        bolt.timer = 0.0f;

        bolt.warningDuration = 0.03f + (float)GetRandomValue(0, 2) / 100.0f;
        bolt.strikeDuration  = 0.05f + (float)GetRandomValue(0, 3) / 100.0f;
        bolt.fadeDuration    = 0.12f + (float)GetRandomValue(0, 10) / 100.0f;

        bolt.intensity = 0.90f + (float)GetRandomValue(0, 20) / 100.0f;
        bolt.flashStrength = 0.75f + (float)GetRandomValue(0, 25) / 100.0f;

        float startX = (float)GetRandomValue(260, 1660);
        Vector2 start = { startX, 40.0f };

        float endX = startX + (float)GetRandomValue(-140, 140);
        float endY = (float)GetRandomValue(740, 1000);
        Vector2 end = { endX, endY };
        bolt.impactPoint = end;

        GenerateMainBolt(bolt, start, end, 6, 90.0f);

        int branchCount = GetRandomValue(2, 5);
        int mainCount = (int)bolt.mainSegments.size();

        for (int b = 0; b < branchCount && mainCount > 8; b++)
        {
            int branchIndex = GetRandomValue(mainCount / 8, (mainCount * 5) / 8);
            Vector2 branchStart = bolt.mainSegments[branchIndex].start;

            Vector2 mainDir = Vector2Normalize(Vector2Subtract(
                bolt.mainSegments[branchIndex].end,
                bolt.mainSegments[branchIndex].start
            ));

            Vector2 perp = { -mainDir.y, mainDir.x };
            float side = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

            Vector2 branchEnd = {
                branchStart.x + perp.x * side * (float)GetRandomValue(70, 150),
                branchStart.y + (float)GetRandomValue(50, 110)
            };

            LightningBranch branch;
            branch.intensity = 0.45f + (float)GetRandomValue(0, 30) / 100.0f;

            SubdivideBranch(branch.segments, branchStart, branchEnd, 3, 32.0f, 1.6f);
            bolt.branches.push_back(branch);
        }

        return;
    }
}

void LightningEffect::GenerateMainBolt(LightningBolt& bolt, Vector2 start, Vector2 end, int generations, float offsetAmount)
{
    SubdivideMain(bolt.mainSegments, start, end, generations, offsetAmount, mainBoltThickness, 14.0f);
}

void LightningEffect::SubdivideMain(std::vector<LightningSegment>& outSegments,
                                    Vector2 start,
                                    Vector2 end,
                                    int generations,
                                    float offsetAmount,
                                    float thickness,
                                    float verticalBias)
{
    if (generations <= 0)
    {
        LightningSegment seg;
        seg.start = start;
        seg.end = end;
        seg.thickness = thickness;
        outSegments.push_back(seg);
        return;
    }

    Vector2 mid = {
        (start.x + end.x) * 0.5f,
        (start.y + end.y) * 0.5f
    };

    Vector2 dir = Vector2Subtract(end, start);
    float dirLen = Vector2Length(dir);

    if (dirLen < 0.001f)
    {
        LightningSegment seg;
        seg.start = start;
        seg.end = end;
        seg.thickness = thickness;
        outSegments.push_back(seg);
        return;
    }

    Vector2 normal = Vector2Normalize(Vector2{ -dir.y, dir.x });

    float lateralOffset = (float)GetRandomValue((int)-offsetAmount, (int)offsetAmount);
    mid.x += normal.x * lateralOffset;
    mid.y += normal.y * lateralOffset * 0.35f;
    mid.y += verticalBias;

    float nextThickness = fmaxf(1.1f, thickness * 0.84f);
    float nextBias = verticalBias * 0.65f;

    SubdivideMain(outSegments, start, mid, generations - 1, offsetAmount * 0.58f, nextThickness, nextBias);
    SubdivideMain(outSegments, mid, end, generations - 1, offsetAmount * 0.52f, nextThickness, nextBias * 0.75f);
}

void LightningEffect::SubdivideBranch(std::vector<LightningSegment>& outSegments,
                                      Vector2 start,
                                      Vector2 end,
                                      int generations,
                                      float offsetAmount,
                                      float thickness)
{
    if (generations <= 0)
    {
        LightningSegment seg;
        seg.start = start;
        seg.end = end;
        seg.thickness = thickness;
        outSegments.push_back(seg);
        return;
    }

    Vector2 mid = {
        (start.x + end.x) * 0.5f,
        (start.y + end.y) * 0.5f
    };

    Vector2 dir = Vector2Subtract(end, start);
    float dirLen = Vector2Length(dir);

    if (dirLen < 0.001f)
    {
        LightningSegment seg;
        seg.start = start;
        seg.end = end;
        seg.thickness = thickness;
        outSegments.push_back(seg);
        return;
    }

    Vector2 normal = Vector2Normalize(Vector2{ -dir.y, dir.x });
    float randomOffset = (float)GetRandomValue((int)-offsetAmount, (int)offsetAmount);

    mid.x += normal.x * randomOffset;
    mid.y += normal.y * randomOffset * 0.25f;
    mid.y += (float)GetRandomValue(8, 20);

    float nextThickness = fmaxf(0.8f, thickness * 0.82f);

    SubdivideBranch(outSegments, start, mid, generations - 1, offsetAmount * 0.55f, nextThickness);
    SubdivideBranch(outSegments, mid, end, generations - 1, offsetAmount * 0.50f, nextThickness);
}

void LightningEffect::Reset() {
    
}