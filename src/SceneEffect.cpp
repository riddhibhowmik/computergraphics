#include "SceneEffect.h"

SceneEffect::SceneEffect(std::string fileName) : Effect("Scene: " + fileName) {
    sceneFileName = fileName;
    background.id = 0;
    layer = -1; // Default to background layer
}

SceneEffect::~SceneEffect() {
    if (background.id != 0) UnloadTexture(background);
}

void SceneEffect::LoadResources(const std::string& projectDir) {
    if (background.id != 0) UnloadTexture(background);
    std::string path = projectDir + "/scene/" + sceneFileName;
    if (FileExists(path.c_str())) {
        background = LoadTexture(path.c_str());
    }
}

void SceneEffect::Update(float dt) {
    // Backgrounds usually don't animate, but we could add pan/zoom later
}

void SceneEffect::Reset() {
    // Nothing to reset for static scenes
}

void SceneEffect::Draw() {
    if (background.id != 0) {
        float fade = GetFadeFactor();
        Rectangle source = {0.0f, 0.0f, (float)background.width, (float)background.height};
        Rectangle dest = {0.0f, 0.0f, 1920.0f, 1080.0f};
        DrawTexturePro(background, source, dest, {0.0f, 0.0f}, 0.0f, Fade(WHITE, fade));
    }
}

std::string SceneEffect::Serialize() {
    return "Effect:Scene;" + SerializeBase() + "File:" + sceneFileName + ";Active:" + std::to_string(isActive);
}

void SceneEffect::Deserialize(const std::string& data) {
    size_t pos = 0;
    std::string token;
    std::string s = data;
    while ((pos = s.find(";")) != std::string::npos) {
        token = s.substr(0, pos);
        if (!DeserializeBaseToken(token)) {
            if (token.find("File:") == 0) sceneFileName = token.substr(5);
            else if (token.find("Active:") == 0) isActive = std::stoi(token.substr(7));
        }
        s.erase(0, pos + 1);
    }
    if (!DeserializeBaseToken(s)) {
        if (s.find("File:") == 0) sceneFileName = s.substr(5);
        else if (s.find("Active:") == 0) isActive = std::stoi(s.substr(7));
    }
    name = "Scene: " + sceneFileName;
}