#pragma once
#include "raylib.h"

struct Particle {

    // variables for kinematics stuff
    Vector2 position;
    Vector2 velocity;

    // variables for rendering stuff
    Color color;
    float size;
    float lifeTime;
    float maxLifeTime;

    // variable for memory pooling
    bool isActive;
};