#pragma once
#include "Effect.h"
#include "raylib.h"

class SceneEffect : public Effect {
public:
    Texture2D background;
    std::string sceneFileName;

    SceneEffect(std::string fileName);
    ~SceneEffect();

    void Update(float dt) override;
    void Draw() override;
    void Reset() override;
    void LoadResources(const std::string& projectDir) override;
    
    std::string Serialize() override;
    void Deserialize(const std::string& data) override;
};
