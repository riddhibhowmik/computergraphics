#pragma once
#include <cmath>

enum class EasingType {
    Linear = 0,
    SineIn,
    SineOut,
    SineInOut,
    QuadIn,
    QuadOut,
    QuadInOut,
    Custom,
    Count // Used for UI iteration
};

class Easing {
public:
    static float Apply(EasingType type, float t) {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        
        switch (type) {
            case EasingType::Linear: return t;
            case EasingType::SineIn: return 1.0f - cos((t * 3.14159265f) / 2.0f);
            case EasingType::SineOut: return sin((t * 3.14159265f) / 2.0f);
            case EasingType::SineInOut: return -(cos(3.14159265f * t) - 1.0f) / 2.0f;
            case EasingType::QuadIn: return t * t;
            case EasingType::QuadOut: return 1.0f - (1.0f - t) * (1.0f - t);
            case EasingType::QuadInOut: return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
            case EasingType::Custom: return t; // Custom logic hook for user scripts
            default: return t;
        }
    }
    
    static const char* GetName(EasingType type) {
        switch (type) {
            case EasingType::Linear: return "Linear";
            case EasingType::SineIn: return "Sine In";
            case EasingType::SineOut: return "Sine Out";
            case EasingType::SineInOut: return "Sine In/Out";
            case EasingType::QuadIn: return "Quad In";
            case EasingType::QuadOut: return "Quad Out";
            case EasingType::QuadInOut: return "Quad In/Out";
            case EasingType::Custom: return "Custom Script...";
            default: return "Unknown";
        }
    }
};
