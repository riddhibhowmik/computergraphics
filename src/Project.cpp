#include "Project.h"
#include <fstream>
#include <iostream>

Project::Project() {
    projectName = "NewProject";
    projectDirectory = "";
    selectedEffectIndex = -1;
}

bool Project::Save(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    file << "ProjectName:" << projectName << "\n";
    for(auto& eff : activeEffects) {
        file << eff->Serialize() << "\n";
    }
    
    file.close();
    return true;
}

bool Project::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    // TODO: Implement parsing of the .qfx custom formatting
    std::string line;
    while(std::getline(file, line)) {
        // Parse logic
    }
    
    file.close();
    return true;
}

void Project::AddEffect(std::shared_ptr<Effect> effect) {
    activeEffects.push_back(effect);
}
