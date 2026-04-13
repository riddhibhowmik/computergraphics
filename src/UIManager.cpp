#include "UIManager.h"
#include "SnowEffect.h"
#include "FireEffect.h"
#include "SparkEffect.h"
#include "Easings.h"
#include <iostream>
#include <sstream>
#include <string>

UIManager::UIManager()
{
}

bool UIManager::Button(Rectangle bounds, const char *text)
{
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    Color btnColor = LIGHTGRAY;

    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        btnColor = GRAY;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            btnColor = DARKGRAY;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            clicked = true;
        }
    }

    DrawRectangleRec(bounds, btnColor);
    DrawRectangleLinesEx(bounds, 1, BLACK);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + bounds.width / 2 - textWidth / 2, bounds.y + bounds.height / 2 - 10, 20, BLACK);

    return clicked;
}

float UIManager::Slider(Rectangle bounds, const char *text, float value, float minValue, float maxValue)
{
    DrawText(text, bounds.x, bounds.y - 20, 20, BLACK);

    DrawRectangleRec(bounds, LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);

    float normalizedValue = (value - minValue) / (maxValue - minValue);
    if (normalizedValue < 0.0f)
        normalizedValue = 0.0f;
    if (normalizedValue > 1.0f)
        normalizedValue = 1.0f;

    Rectangle handle = {bounds.x + normalizedValue * bounds.width - 5, bounds.y - 2, 10, bounds.height + 4};

    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, bounds) || CheckCollisionPointRec(mousePoint, handle))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            float mouseX = mousePoint.x - bounds.x;
            normalizedValue = mouseX / bounds.width;
            if (normalizedValue < 0.0f)
                normalizedValue = 0.0f;
            if (normalizedValue > 1.0f)
                normalizedValue = 1.0f;
            value = minValue + normalizedValue * (maxValue - minValue);
        }
    }

    DrawRectangleRec(handle, DARKGRAY);
    return value;
}

bool UIManager::FloatBox(Rectangle bounds, float *value, int id)
{
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        activeTextBox = id;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", *value);
        textInputBuffer = buf;
    }
    else if (!isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && activeTextBox == id)
    {
        activeTextBox = -1;
        try
        {
            *value = std::stof(textInputBuffer);
        }
        catch (...)
        {
        }
    }

    if (activeTextBox == id)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= '0' && key <= '9') || key == '.' || key == '-')
            {
                textInputBuffer += (char)key;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !textInputBuffer.empty())
            textInputBuffer.pop_back();
        if (IsKeyPressed(KEY_ENTER))
        {
            activeTextBox = -1;
            try
            {
                *value = std::stof(textInputBuffer);
            }
            catch (...)
            {
            }
        }
    }

    DrawRectangleRec(bounds, activeTextBox == id ? RAYWHITE : LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 1, activeTextBox == id ? BLUE : DARKGRAY);

    std::string displayText = (activeTextBox == id) ? textInputBuffer + "_" : TextFormat("%.2f", *value);
    DrawText(displayText.c_str(), bounds.x + 5, bounds.y + bounds.height / 2 - 10, 20, BLACK);

    return activeTextBox == id;
}

bool UIManager::IntBox(Rectangle bounds, int *value, int id)
{
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        activeTextBox = id;
        textInputBuffer = std::to_string(*value);
    }
    else if (!isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && activeTextBox == id)
    {
        activeTextBox = -1;
        try
        {
            *value = std::stoi(textInputBuffer);
        }
        catch (...)
        {
        }
    }

    if (activeTextBox == id)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= '0' && key <= '9') || key == '-')
            {
                textInputBuffer += (char)key;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !textInputBuffer.empty())
            textInputBuffer.pop_back();
        if (IsKeyPressed(KEY_ENTER))
        {
            activeTextBox = -1;
            try
            {
                *value = std::stoi(textInputBuffer);
            }
            catch (...)
            {
            }
        }
    }

    DrawRectangleRec(bounds, activeTextBox == id ? RAYWHITE : LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 1, activeTextBox == id ? BLUE : DARKGRAY);

    std::string displayText = (activeTextBox == id) ? textInputBuffer + "_" : TextFormat("%d", *value);
    DrawText(displayText.c_str(), bounds.x + 5, bounds.y + bounds.height / 2 - 10, 20, BLACK);

    return activeTextBox == id;
}

void UIManager::UpdateAndDraw(Project &currentProject)
{
    DrawToolbar();
    DrawTimeline(currentProject);
    DrawSidebar(currentProject); // Draw sidebar last so it stays on top of timeline if needed
}

void UIManager::DrawToolbar()
{
    // Toolbar on top of screen
    DrawRectangle(0, 0, 1920, 40, DARKGRAY);
    DrawText("File: Create New | Load | Save | Save As | Open Location | Import | Export | Options | Exit", 20, 10, 20, RAYWHITE);
}

void UIManager::DrawSidebar(Project &currentProject)
{
    // Right sidebar: 480 x 900
    DrawRectangle(1920 - 480, 40, 480, 900, Fade(RAYWHITE, 0.95f));
    DrawRectangleLines(1920 - 480, 40, 480, 900, DARKGRAY);

    DrawText("Usable Effects", 1920 - 480 + 20, 60, 24, BLACK);
    DrawLine(1920 - 480 + 20, 90, 1920 - 20, 90, DARKGRAY);

    if (Button({1920 - 480 + 20, 100, 440, 40}, "+ Add Snow Effect"))
    {
        currentProject.AddEffect(std::make_shared<SnowEffect>());
        currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
    }
    if (Button({1920 - 480 + 20, 150, 440, 40}, "+ Add Fire Effect"))
    {
        currentProject.AddEffect(std::make_shared<FireEffect>());
        currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
    }
    if (Button({1920 - 480 + 20, 200, 440, 40}, "+ Add Spark Effect"))
    {
        currentProject.AddEffect(std::make_shared<SparkEffect>());
        currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
    }
    if (Button({1920 - 480 + 20, 250, 440, 40}, "+ Add custom script"))
    {
        // Placeholder for future script loading
    }

    // Properties area
    DrawLine(1920 - 480 + 20, 210, 1920 - 20, 210, DARKGRAY);
    DrawText("Properties", 1920 - 480 + 20, 220, 24, BLACK);

    if (currentProject.selectedEffectIndex >= 0 && currentProject.selectedEffectIndex < currentProject.activeEffects.size())
    {
        auto effect = currentProject.activeEffects[currentProject.selectedEffectIndex];
        DrawText(TextFormat("Editing: %s", effect->name.c_str()), 1920 - 480 + 20, 260, 20, DARKBLUE);

        // Use dynamic cast to edit SnowEffect specifically
        if (auto snow = std::dynamic_pointer_cast<SnowEffect>(effect))
        {
            // Speed Property
            snow->speed = Slider({1920 - 480 + 20, 310, 300, 20}, "Speed", snow->speed, 0.0f, 1000.0f);
            FloatBox({1920 - 480 + 340, 300, 100, 30}, &snow->speed, 1);

            // Density Property
            snow->density = Slider({1920 - 480 + 20, 370, 300, 20}, "Density", snow->density, 10.0f, 5000.0f);
            FloatBox({1920 - 480 + 340, 360, 100, 30}, &snow->density, 2);

            // Direction Property
            snow->directionAngle = Slider({1920 - 480 + 20, 430, 300, 20}, "Angle", snow->directionAngle, -180.0f, 360.0f);
            FloatBox({1920 - 480 + 340, 420, 100, 30}, &snow->directionAngle, 3);

            // Seed Property
            float seedFloat = (float)snow->seed;
            seedFloat = Slider({1920 - 480 + 20, 490, 300, 20}, "Seed", seedFloat, 0.0f, 100.0f);
            if (activeTextBox != 4)
                snow->seed = (int)seedFloat; // Only update from slider if not typing
            IntBox({1920 - 480 + 340, 480, 100, 30}, &snow->seed, 4);

            // Easing Type Property
            DrawText("Easing Type:", 1920 - 480 + 20, 530, 20, BLACK);
            DrawText(Easing::GetName((EasingType)snow->easingType), 1920 - 480 + 160, 530, 20, DARKBLUE);
            if (Button({1920 - 480 + 20, 560, 100, 30}, "< Prev"))
            {
                snow->easingType--;
                if (snow->easingType < 0)
                    snow->easingType = (int)EasingType::Count - 1;
            }
            if (Button({1920 - 480 + 130, 560, 100, 30}, "Next >"))
            {
                snow->easingType++;
                if (snow->easingType >= (int)EasingType::Count)
                    snow->easingType = 0;
            }
            if (Button({1920 - 480 + 240, 560, 140, 30}, "+ Custom Easing"))
            {
                snow->easingType = (int)EasingType::Custom;
            }

            // Re-initialize particles button
            if (Button({1920 - 480 + 20, 610, 440, 40}, "Apply Density & Seed"))
            {
                snow->SetSeed(snow->seed);
                snow->InitParticles();
            }

            // Toggle Active
            DrawText(TextFormat("Active: %s", snow->isActive ? "Yes" : "No"), 1920 - 480 + 20, 670, 20, BLACK);
            if (Button({1920 - 480 + 200, 665, 80, 30}, "Toggle"))
                snow->isActive = !snow->isActive;
        }
        else if (effect->name == "Fire")
        {
            auto fire = std::static_pointer_cast<FireEffect>(effect);
            DrawText("Fire Settings", 1920 - 480 + 20, 260, 20, BLACK);
            // wind slider
            fire->wind = Slider({1920 - 480 + 20, 320, 440, 20},
                                TextFormat("wind force: %.1f", fire->wind), fire->wind, -2500.0f, 2500.0f);
            // spawnrate slider
            fire->spawnRate = (int)Slider({1920 - 480 + 20, 380, 440, 20},
                                          TextFormat("spawn rate: %d", fire->spawnRate), (float)fire->spawnRate, 10.0f, 200.0f);
            // toggle
            DrawText(TextFormat("active: %s", fire->isActive ? "yes" : "no"), 1920 - 480 + 20, 440, 20, BLACK);
            if (Button({1920 - 480 + 200, 435, 80, 30}, "toggle"))
                fire->isActive = !fire->isActive;
        }
        else if (effect->name == "Spark") {
            auto sparks = std::static_pointer_cast<SparkEffect>(effect);
            DrawText("spark settings", 1920 - 480 + 20, 260, 20, BLACK);

            // spawnrate slider
            sparks->spawnRate = (int)Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("spawn rate: %d", sparks->spawnRate), (float)sparks->spawnRate, 1.0f, 50.0f);
            // gravity slider
            sparks->gravity = Slider({1920 - 480 + 20, 380, 440, 20},
                                TextFormat("gravity: %.1f", sparks->gravity), sparks->gravity, 100.0f, 3000.0f);
            // toggle
            DrawText(TextFormat("active: %s", sparks->isActive ? "yes" : "no"), 1920 - 480 + 20, 440, 20, BLACK);
            if (Button({1920 - 480 + 200, 435, 80, 30}, "toggle"))
                sparks->isActive = !sparks->isActive;
        }
        else
        {
            DrawText("No effect selected.", 1920 - 480 + 20, 260, 20, GRAY);
        }
    }
}

void UIManager::DrawTimeline(Project &currentProject)
{
    // Bottom side scroll timeline: 1920 x 180
    DrawRectangle(0, 1080 - 180, 1920, 180, Fade(GRAY, 0.95f));
    DrawRectangleLines(0, 1080 - 180, 1920, 180, DARKGRAY);
    DrawText("Timeline / Active Effects", 20, 1080 - 160, 24, BLACK);

    // Draw each active effect as a block
    float xOffset = 20.0f;
    float yOffset = 1080 - 120.0f;

    for (int i = 0; i < currentProject.activeEffects.size(); i++)
    {
        Rectangle blockBounds = {xOffset, yOffset, 150, 80};

        bool isSelected = (currentProject.selectedEffectIndex == i);
        if (isSelected)
        {
            DrawRectangleRec(blockBounds, LIGHTGRAY);
            DrawRectangleLinesEx(blockBounds, 3, BLUE);
        }

        if (Button(blockBounds, currentProject.activeEffects[i]->name.c_str()))
        {
            currentProject.selectedEffectIndex = i;
        }

        xOffset += 160.0f;
    }
}