#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Effect.h"

class Project {
public:
    std::string projectName;
    std::string projectDirectory;
    std::vector<std::shared_ptr<Effect>> activeEffects;
    int selectedEffectIndex;
    
    Project();
    
    // Save to and Load from .qfx file
    bool Save(const std::string& filepath);
    bool Load(const std::string& filepath);
    
    void AddEffect(std::shared_ptr<Effect> effect);
};
