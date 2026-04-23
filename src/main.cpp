#include "raylib.h"
#include <memory>
#include "Project.h"
#include "UIManager.h"
#include "SnowEffect.h"
#include "FireEffect.h"
#include "SparkEffect.h"
#include "NebulaEffect.h"
#include "BlackHoleEffect.h"
#include "RainEffect.h"
#include "LightningEffect.h"

const char* bloomShaderCode = R"GLSL(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
// brightness threshold
const float threshold = 0.2; 
// intensity
const float amount = 2.5;
const float blurSize = 0.006;

// simple bloom shader
void main() {
    vec4 base = texture(texture0, fragTexCoord);
    vec3 glow = vec3(0.0);

    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            vec2 offset = vec2(i, j) * blurSize;
            vec4 sample = texture(texture0, fragTexCoord + offset);
            float brightness = dot(sample.rgb, vec3(0.2126, 0.7152, 0.0722));
            if (brightness > threshold) {
                glow += sample.rgb;
            }
        }
        // average samples
        glow /= 9;
        final = vec4(base.rgb + (glow * amount), base.a) * colDiffuse * fragColor;
    }
}
)GLSL";

int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "particle vfx engine");
    //SetTargetFPS(60);

    // load bloom shader
    Shader bloom = LoadShaderFromMemory(0, bloomShaderCode);

    // make the canvas for effects, we gonna apply bloom here
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    Project project;
    UIManager uiManager;

    //project.AddEffect(std::make_shared<SnowEffect>());
    //project.AddEffect(std::make_shared<FireEffect>());
    //project.AddEffect(std::make_shared<SparkEffect>());
    //project.AddEffect(std::make_shared<NebulaEffect>());
    //project.AddEffect(std::make_shared<BlackHoleEffect>());
    //project.AddEffect(std::make_shared<RainEffect>());
    //project.AddEffect(std::make_shared<LightningEffect>());

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        for (auto& effect : project.activeEffects)
            if(effect->isActive)
                effect->Update(dt);

        float lightningFlash = 0.0f;

        for (auto& effect: project.activeEffects) {
            if (effect->isActive && effect->name == "Lightning") {
                auto lightning = std::static_pointer_cast<LightningEffect>(effect);
                lightningFlash = std::max(lightningFlash, lightning->GetGlobalFlashAlpha());
            }
        }

        for (auto& effect : project.activeEffects) {
            if (effect->isActive && effect->name == "Rain") {
                auto rain = std::static_pointer_cast<RainEffect>(effect);
                rain->SetLightningFlashAlpha(lightningFlash);
            }
        }

        // first render pass, draw all effects to target texture on canvas
        BeginTextureMode(target);
        ClearBackground(BLACK);

        for (auto& effect : project.activeEffects)
            if(effect->isActive)
                effect->Draw();

        EndTextureMode();

        // second rende rpass, draw canvas to screen with bloom shader
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(bloom);

        Rectangle source = {0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height};
        Vector2 position = {0.0f, 0.0f};
        DrawTextureRec(target.texture, source, position, WHITE);

        EndShaderMode();

        // render pass 3, draw ui on top of everything
        uiManager.UpdateAndDraw(project);
        EndDrawing();
    }
    UnloadShader(bloom);
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}