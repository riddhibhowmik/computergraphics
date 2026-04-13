#include "raylib.h"
#include <memory>
#include "Project.h"
#include "UIManager.h"
#include "SnowEffect.h"
#include "FireEffect.h"
#include "SparkEffect.h"

int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "particle vfx engine");
    SetTargetFPS(60);

    Project project;
    UIManager uiManager;

    project.AddEffect(std::make_shared<SnowEffect>());
    project.AddEffect(std::make_shared<FireEffect>());
    project.AddEffect(std::make_shared<SparkEffect>());

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        for (auto& effect : project.activeEffects)
            if(effect->isActive)
                effect->Update(dt);

        BeginDrawing();
        ClearBackground(BLACK);

        for (auto& effect : project.activeEffects)
            if(effect->isActive)
                effect->Draw();

        uiManager.UpdateAndDraw(project);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}