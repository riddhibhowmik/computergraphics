#include "RainEffect.h"
#include "raymath.h"
#include <cmath>

RainEffect::RainEffect() : Effect("Rain")
{
    baseWindX = -160.0f;
    groundY = 980.0f;
    stormIntensity = 1.0f;
    gustStrength = 1.0f;
    lightningFlashInfluence = 0.45f;
    depthContrast = 1.0f;
    burstStrength = 1.0f;

    gustTime = 0.0f;
    lightningFlashAlpha = 0.0f;

    farCount = 900;
    midCount = 700;
    nearCount = 350;
    maxSplashes = 900;

    dropsFar.resize(farCount);
    dropsMid.resize(midCount);
    dropsNear.resize(nearCount);
    splashes.resize(maxSplashes);

    for (int i = 0; i < farCount; i++)
    {
        dropsFar[i].active = true;
        ResetDrop(dropsFar[i], true, 0.15f, 0.35f);
    }

    for (int i = 0; i < midCount; i++)
    {
        dropsMid[i].active = true;
        ResetDrop(dropsMid[i], true, 0.45f, 0.70f);
    }

    for (int i = 0; i < nearCount; i++)
    {
        dropsNear[i].active = true;
        ResetDrop(dropsNear[i], true, 0.80f, 1.00f);
    }

    for (int i = 0; i < maxSplashes; i++)
    {
        splashes[i].active = false;
    }
}

RainEffect::~RainEffect() {}

void RainEffect::ResetDrop(RainDrop& drop, bool randomY, float depthMin, float depthMax)
{
    float depthT = (float)GetRandomValue(0, 1000) / 1000.0f;
    drop.depth = depthMin + (depthMax - depthMin) * depthT;

    drop.pos.x = (float)GetRandomValue(-100, 2020);

    if (randomY)
        drop.pos.y = (float)GetRandomValue(-1080, 1080);
    else
        drop.pos.y = (float)GetRandomValue(-250, -20);

    float baseSpeed = (900.0f + 650.0f * drop.depth) * stormIntensity;
    float xVariation = (float)GetRandomValue(-18, 18);

    drop.vel.x = baseWindX + xVariation;
    drop.vel.y = baseSpeed;

    drop.length = (float)GetRandomValue(10, 22);
    drop.thickness = (float)GetRandomValue(8, 14) / 10.0f;
    drop.alpha = (float)GetRandomValue(60, 145);
    drop.active = true;
}

void RainEffect::SpawnSplash(const Vector2& pos, float strength)
{
    int created = 0;

    for (int i = 0; i < maxSplashes && created < 4; i++)
    {
        if (splashes[i].active) continue;

        RainSplash& s = splashes[i];
        s.active = true;
        s.pos = pos;
        s.radius = ((float)GetRandomValue(1, 3)) * (0.7f + strength * 0.4f);
        s.maxLife = 0.10f + ((float)GetRandomValue(0, 10) / 100.0f);
        s.life = s.maxLife;
        s.alpha = 110.0f + strength * 65.0f + (float)GetRandomValue(0, 25);

        float vx = (float)GetRandomValue(-55, 55);
        float vy = (float)GetRandomValue(-210, -120);

        if (created == 2 || created == 3)
        {
            vx *= 1.6f;
            vy *= 0.7f;
        }

        s.vel = { vx, vy };
        created++;
    }
}

void RainEffect::UpdateDrop(RainDrop& drop, float dt, float gustWind)
{
    if (!drop.active) return;

    float targetWind = baseWindX + gustWind;
    drop.vel.x += (targetWind - drop.vel.x) * (2.2f * dt);

    drop.pos.x += drop.vel.x * dt;
    drop.pos.y += drop.vel.y * dt;

    drop.vel.y += 90.0f * dt * (0.5f + drop.depth);

    if (drop.pos.x < -140) drop.pos.x = 2060;
    if (drop.pos.x > 2060) drop.pos.x = -140;

    if (drop.pos.y >= groundY)
    {
        SpawnSplash({ drop.pos.x, groundY }, drop.depth);

        if (drop.depth < 0.40f) ResetDrop(drop, false, 0.15f, 0.35f);
        else if (drop.depth < 0.75f) ResetDrop(drop, false, 0.45f, 0.70f);
        else ResetDrop(drop, false, 0.80f, 1.00f);
        return;
    }

    if (drop.pos.y > 1120)
    {
        if (drop.depth < 0.40f) ResetDrop(drop, false, 0.15f, 0.35f);
        else if (drop.depth < 0.75f) ResetDrop(drop, false, 0.45f, 0.70f);
        else ResetDrop(drop, false, 0.80f, 1.00f);
    }
}

void RainEffect::DrawDrop(const RainDrop& d) const
{
    Vector2 dir = Vector2Normalize(d.vel);

    float depthFactor = d.depth * depthContrast;
    float length = d.length * (0.7f + depthFactor * 1.3f);
    float thickness = d.thickness * (0.6f + depthFactor * 1.4f);

    Vector2 tail = {
        d.pos.x - dir.x * length,
        d.pos.y - dir.y * length
    };

    float flashBoost = 1.0f + (lightningFlashAlpha / 120.0f) * lightningFlashInfluence;
    float alphaValue = d.alpha * (0.4f + depthFactor * 1.2f) * flashBoost;
    unsigned char a = (unsigned char)fminf(255.0f, alphaValue);

    Color rainColor = {
        (unsigned char)(200 + d.depth * 20.0f),
        (unsigned char)(210 + d.depth * 20.0f),
        (unsigned char)(220 + d.depth * 30.0f),
        a
    };

    DrawLineEx(tail, d.pos, thickness, rainColor);

    if (d.depth > 0.82f)
    {
        Color headGlow = { 220, 230, 240, (unsigned char)(a * 0.18f) };
        DrawCircleV(d.pos, thickness * 1.1f, headGlow);
    }
}

void RainEffect::Update(float dt)
{
    gustTime += dt;

    float gustWaveA = sinf(gustTime * 0.70f);
    float gustWaveB = sinf(gustTime * 1.85f + 1.2f);

    float burstMask = (sinf(gustTime * 0.32f) > 0.82f) ? 1.0f : 0.0f;
    float burstWind = 110.0f * burstStrength * burstMask;

    float gustWind = ((gustWaveA * 38.0f) + (gustWaveB * 22.0f)) * stormIntensity * gustStrength + burstWind;

    for (int i = 0; i < farCount; i++)
        UpdateDrop(dropsFar[i], dt, gustWind * 0.50f);

    for (int i = 0; i < midCount; i++)
        UpdateDrop(dropsMid[i], dt, gustWind * 0.85f);

    for (int i = 0; i < nearCount; i++)
        UpdateDrop(dropsNear[i], dt, gustWind * 1.20f);

    for (int i = 0; i < maxSplashes; i++)
    {
        if (!splashes[i].active) continue;

        RainSplash& s = splashes[i];
        s.life -= dt;

        if (s.life <= 0.0f)
        {
            s.active = false;
            continue;
        }

        s.pos.x += s.vel.x * dt;
        s.pos.y += s.vel.y * dt;
        s.vel.y += 760.0f * dt;
    }
}

void RainEffect::Draw()
{
    BeginBlendMode(BLEND_ALPHA);

    for (int i = 0; i < farCount; i++)
        if (dropsFar[i].active) DrawDrop(dropsFar[i]);

    for (int i = 0; i < midCount; i++)
        if (dropsMid[i].active) DrawDrop(dropsMid[i]);

    for (int i = 0; i < nearCount; i++)
        if (dropsNear[i].active) DrawDrop(dropsNear[i]);

    for (int i = 0; i < maxSplashes; i++)
    {
        if (!splashes[i].active) continue;

        const RainSplash& s = splashes[i];
        float t = s.life / s.maxLife;
        float flashBoost = 1.0f + (lightningFlashAlpha / 120.0f) * lightningFlashInfluence;
        unsigned char a = (unsigned char)fminf(255.0f, s.alpha * t * flashBoost);

        Color splashColor = { 220, 230, 240, a };
        DrawCircleV(s.pos, s.radius * t, splashColor);

        Vector2 splashLeft = { s.pos.x - 2.0f * t, s.pos.y };
        Vector2 splashRight = { s.pos.x + 2.0f * t, s.pos.y };
        DrawLineEx(splashLeft, splashRight, 1.0f, Color{220, 230, 240, (unsigned char)(a * 0.65f)});
    }

    EndBlendMode();
}