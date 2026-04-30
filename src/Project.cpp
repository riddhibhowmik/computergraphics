#include "Project.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include "SnowEffect.h"
#include "FireEffect.h"
#include "SparkEffect.h"
#include "NebulaEffect.h"
#include "BlackHoleEffect.h"
#include "SceneEffect.h"

Project::Project() {
    projectName = "NewProject";
    projectDirectory = "";
    selectedEffectIndex = -1;
    hasUnsavedChanges = false;
    
    currentTime = 0.0f;
    totalDuration = 30.0f; // Default timeline length
    isPlaying = false;
}

bool Project::Save(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    file << "ProjectName:" << projectName << "\n";
    file << "Duration:" << totalDuration << "\n";
    for(auto& eff : activeEffects) {
        file << eff->Serialize() << "\n";
    }
    
    file.close();
    hasUnsavedChanges = false;
    return true;
}

bool Project::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    activeEffects.clear();
    
    std::string line;
    while(std::getline(file, line)) {
        if (line.rfind("ProjectName:", 0) == 0) {
            projectName = line.substr(12);
        } else if (line.rfind("Duration:", 0) == 0) {
            totalDuration = std::stof(line.substr(9));
        } else if (line.rfind("Effect:", 0) == 0) {
            std::string effectName = line.substr(7, line.find(";") - 7);
            std::shared_ptr<Effect> effect = nullptr;
            
            if (effectName == "Snow") effect = std::make_shared<SnowEffect>();
            else if (effectName == "Fire") effect = std::make_shared<FireEffect>();
            else if (effectName == "Spark") effect = std::make_shared<SparkEffect>();
            else if (effectName == "Nebula") effect = std::make_shared<NebulaEffect>();
            else if (effectName == "Black Hole") effect = std::make_shared<BlackHoleEffect>();
            else if (effectName == "Scene") effect = std::make_shared<SceneEffect>("");
            
            if (effect) {
                effect->Deserialize(line);
                if (!projectDirectory.empty()) {
                    effect->LoadResources(projectDirectory);
                }
                activeEffects.push_back(effect);
            }
        }
    }
    
    file.close();
    SortEffectsByLayer();
    hasUnsavedChanges = false;
    return true;
}

void Project::AddEffect(std::shared_ptr<Effect> effect) {
    if (!projectDirectory.empty()) {
        effect->LoadResources(projectDirectory);
    }
    // Give it a default layer on top
    int maxLayer = 0;
    for (const auto& eff : activeEffects) {
        if (eff->layer > maxLayer) maxLayer = eff->layer;
    }
    effect->layer = maxLayer + 1;
    
    activeEffects.push_back(effect);
    SortEffectsByLayer();
    hasUnsavedChanges = true;
}

void Project::SortEffectsByLayer() {
    // Sort in ascending order so higher layers draw last (on top)
    std::stable_sort(activeEffects.begin(), activeEffects.end(), [](const std::shared_ptr<Effect>& a, const std::shared_ptr<Effect>& b) {
        return a->layer < b->layer;
    });
}

void Project::UpdatePlayback(float dt) {
    if (isPlaying) {
        float nextTime = currentTime + dt;
        if (nextTime > totalDuration) {
            SetTime(0.0f); // loop back to start and rebuild state
            return;
        }
        
        for (auto& effect : activeEffects) {
            if (effect->isActive && currentTime >= effect->startTime && currentTime <= effect->endTime) {
                effect->timelineTime = currentTime;
                effect->Update(dt);
            }
        }
        currentTime = nextTime;
    }
}

void Project::SetTime(float newTime) {
    if (newTime < currentTime) {
        // If we seek backwards, we must fully reset and simulate from 0 to newTime
        currentTime = newTime;
        RebuildState();
    } else if (newTime > currentTime) {
        // Fast forward
        float simTime = currentTime;
        while (simTime < newTime) {
            float dt = std::min(1.0f / 60.0f, newTime - simTime);
            for (auto& effect : activeEffects) {
                if (effect->isActive && simTime >= effect->startTime && simTime <= effect->endTime) {
                    effect->timelineTime = simTime;
                    effect->Update(dt);
                }
            }
            simTime += dt;
        }
        currentTime = newTime;
    }
}

void Project::RebuildState() {
    for (auto& effect : activeEffects) {
        effect->Reset();
    }
    
    float targetTime = currentTime;
    float simTime = 0.0f;
    
    while (simTime < targetTime) {
        float dt = std::min(1.0f / 60.0f, targetTime - simTime);
        for (auto& effect : activeEffects) {
            if (effect->isActive && simTime >= effect->startTime && simTime <= effect->endTime) {
                effect->timelineTime = simTime;
                effect->Update(dt);
            }
        }
        simTime += dt;
    }
}

void Project::Draw() {
    for (auto& effect : activeEffects) {
        if (effect->isActive && currentTime >= effect->startTime && currentTime <= effect->endTime) {
            effect->timelineTime = currentTime;
            effect->Draw();
        }
    }
}
