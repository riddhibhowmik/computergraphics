#include "UIManager.h"
#include "SnowEffect.h"
#include "FireEffect.h"
#include "SparkEffect.h"
#include "NebulaEffect.h"
#include "BlackHoleEffect.h"
#include "RainEffect.h"
#include "LightningEffect.h"
#include "SceneEffect.h"
#include "Easings.h"
#include "FileDialog.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>

UIManager::UIManager()
{
    currentState = AppState::MainMenu;
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

std::string UIManager::GetSaveDirectory() {
    std::string path;
    if (const char* env_p = std::getenv("USERPROFILE")) {
        path = std::string(env_p) + "/Documents/ParticleForge";
    } else {
        path = "ParticleForge";
    }
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    return path;
}

bool UIManager::ProjectExists(const std::string& name) {
    std::string path = GetSaveDirectory() + "/" + name + "/" + name + ".qfx";
    return std::filesystem::exists(path);
}

std::string UIManager::GetUniqueProjectName(const std::string& baseName) {
    if (!ProjectExists(baseName)) return baseName;
    int counter = 1;
    while (ProjectExists(baseName + "(" + std::to_string(counter) + ")")) {
        counter++;
    }
    return baseName + "(" + std::to_string(counter) + ")";
}

void UIManager::CreateNewProject(Project& currentProject, const std::string& name) {
    currentProject = Project(); // Reset project
    currentProject.projectName = name;
    currentProject.projectDirectory = GetSaveDirectory() + "/" + name;
    std::filesystem::create_directories(currentProject.projectDirectory);
    std::filesystem::create_directories(currentProject.projectDirectory + "/sprite");
    std::filesystem::create_directories(currentProject.projectDirectory + "/scene");
    currentProject.Save(currentProject.projectDirectory + "/" + name + ".qfx");
    currentState = AppState::Editing;
}

void UIManager::DrawMainMenu(Project& currentProject) {
    DrawText("ParticleForge", 1920/2 - MeasureText("ParticleForge", 60)/2, 300, 60, RAYWHITE);
    
    if (Button({1920/2 - 150.0f, 450, 300, 60}, "Create New Project")) {
        newProjectNameBuffer = "";
        currentState = AppState::NewProjectPrompt;
    }
    if (Button({1920/2 - 150.0f, 530, 300, 60}, "Load Project")) {
        std::string file = FileDialog::OpenFile("Project Files (*.qfx)\0*.qfx\0All Files (*.*)\0*.*\0", GetSaveDirectory().c_str());
        if (!file.empty()) {
            currentProject = Project();
            currentProject.Load(file);
            std::filesystem::path p(file);
            currentProject.projectName = p.stem().string();
            currentProject.projectDirectory = p.parent_path().string();
            currentState = AppState::Editing;
        }
    }
    if (Button({1920/2 - 150.0f, 610, 300, 60}, "Exit")) {
        currentState = AppState::Exit;
    }
}

void UIManager::DrawNewProjectPrompt(Project& currentProject) {
    DrawRectangle(1920/2 - 300, 1080/2 - 150, 600, 300, RAYWHITE);
    DrawRectangleLines(1920/2 - 300, 1080/2 - 150, 600, 300, DARKGRAY);
    DrawText("Enter Project Name:", 1920/2 - MeasureText("Enter Project Name:", 30)/2, 1080/2 - 100, 30, BLACK);
    
    Rectangle tb = {1920/2 - 250.0f, 1080/2 - 40.0f, 500, 50};
    DrawRectangleRec(tb, LIGHTGRAY);
    DrawRectangleLinesEx(tb, 1, BLACK);
    DrawText(newProjectNameBuffer.c_str(), tb.x + 10, tb.y + 15, 20, BLACK);
    
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125)) {
            newProjectNameBuffer += (char)key;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !newProjectNameBuffer.empty()) {
        newProjectNameBuffer.pop_back();
    }
    
    if (Button({1920/2 - 150.0f, 1080/2 + 50.0f, 120, 40}, "Create")) {
        std::string name = newProjectNameBuffer;
        if (name.empty()) {
            name = "NewProject1";
            if (ProjectExists(name)) {
                pendingProjectName = name;
                currentState = AppState::NameExistsPrompt;
                return;
            }
        } else {
            if (ProjectExists(name)) {
                pendingProjectName = name;
                currentState = AppState::NameExistsPrompt;
                return;
            }
        }
        CreateNewProject(currentProject, name);
    }
    if (Button({1920/2 + 30.0f, 1080/2 + 50.0f, 120, 40}, "Cancel")) {
        currentState = AppState::MainMenu;
    }
}

void UIManager::DrawNameExistsPrompt(Project& currentProject) {
    DrawRectangle(1920/2 - 350, 1080/2 - 150, 700, 300, RAYWHITE);
    DrawRectangleLines(1920/2 - 350, 1080/2 - 150, 700, 300, DARKGRAY);
    
    DrawText("Project name exists.", 1920/2 - MeasureText("Project name exists.", 30)/2, 1080/2 - 100, 30, BLACK);
    std::string uniqueName = GetUniqueProjectName(pendingProjectName);
    const char* promptText = TextFormat("Save as '%s'?", uniqueName.c_str());
    DrawText(promptText, 1920/2 - MeasureText(promptText, 30)/2, 1080/2 - 50, 30, BLACK);
    
    if (Button({1920/2 - 150.0f, 1080/2 + 50.0f, 120, 40}, "Yes")) {
        CreateNewProject(currentProject, uniqueName);
    }
    if (Button({1920/2 + 30.0f, 1080/2 + 50.0f, 120, 40}, "No")) {
        currentState = AppState::NewProjectPrompt;
    }
}

void UIManager::DrawPromptSaveBeforeNew(Project& currentProject) {
    DrawRectangle(1920/2 - 300, 1080/2 - 150, 600, 300, RAYWHITE);
    DrawRectangleLines(1920/2 - 300, 1080/2 - 150, 600, 300, DARKGRAY);
    DrawText("Save current project first?", 1920/2 - MeasureText("Save current project first?", 30)/2, 1080/2 - 80, 30, BLACK);
    
    if (Button({1920/2 - 200.0f, 1080/2 + 50.0f, 100, 40}, "Yes")) {
        currentProject.Save(currentProject.projectDirectory + "/" + currentProject.projectName + ".qfx");
        newProjectNameBuffer = "";
        currentState = AppState::NewProjectPrompt;
    }
    if (Button({1920/2 - 50.0f, 1080/2 + 50.0f, 100, 40}, "No")) {
        newProjectNameBuffer = "";
        currentState = AppState::NewProjectPrompt;
    }
    if (Button({1920/2 + 100.0f, 1080/2 + 50.0f, 100, 40}, "Cancel")) {
        currentState = AppState::Editing;
    }
}

void UIManager::DrawPromptSaveBeforeExit(Project& currentProject) {
    DrawRectangle(1920/2 - 300, 1080/2 - 150, 600, 300, RAYWHITE);
    DrawRectangleLines(1920/2 - 300, 1080/2 - 150, 600, 300, DARKGRAY);
    DrawText("Save before exit?", 1920/2 - MeasureText("Save before exit?", 30)/2, 1080/2 - 80, 30, BLACK);
    
    if (Button({1920/2 - 200.0f, 1080/2 + 50.0f, 100, 40}, "Yes")) {
        currentProject.Save(currentProject.projectDirectory + "/" + currentProject.projectName + ".qfx");
        currentState = AppState::Exit;
    }
    if (Button({1920/2 - 50.0f, 1080/2 + 50.0f, 100, 40}, "No")) {
        currentState = AppState::Exit;
    }
    if (Button({1920/2 + 100.0f, 1080/2 + 50.0f, 100, 40}, "Cancel")) {
        currentState = AppState::Editing;
    }
}

void UIManager::UpdateAndDraw(Project &currentProject, ExportManager &exporter)
{
    switch(currentState) {
        case AppState::MainMenu: DrawMainMenu(currentProject); break;
        case AppState::NewProjectPrompt: DrawNewProjectPrompt(currentProject); break;
        case AppState::NameExistsPrompt: DrawNameExistsPrompt(currentProject); break;
        case AppState::PromptSaveBeforeNew: DrawPromptSaveBeforeNew(currentProject); break;
        case AppState::PromptSaveBeforeExit: DrawPromptSaveBeforeExit(currentProject); break;
        case AppState::Editing:
            DrawToolbar(currentProject, exporter);
            DrawTimeline(currentProject);
            DrawSidebar(currentProject); 
            
            if (exporter.isExporting) {
                DrawRectangle(0, 0, 1920, 1080, Fade(BLACK, 0.8f));
                DrawText("Exporting to MP4...", 1920/2 - 200, 1080/2 - 50, 40, RAYWHITE);
                DrawRectangle(1920/2 - 250, 1080/2 + 20, 500, 30, DARKGRAY);
                float progress = exporter.currentExportTime / currentProject.totalDuration;
                DrawRectangle(1920/2 - 250, 1080/2 + 20, 500 * progress, 30, GREEN);
                DrawText(TextFormat("%d%%", (int)(progress * 100)), 1920/2 - 20, 1080/2 + 25, 20, RAYWHITE);
            }
            break;
        case AppState::Exit: break;
    }
}

void UIManager::DrawToolbar(Project &currentProject, ExportManager &exporter)
{
    DrawRectangle(0, 0, 1920, 40, DARKGRAY);
    DrawText("File:", 20, 10, 20, RAYWHITE);
    DrawFPS(1920 - 120, 10);
    
    if (Button({80, 5, 120, 30}, "Create New")) {
        if (currentProject.hasUnsavedChanges) {
            currentState = AppState::PromptSaveBeforeNew;
        } else {
            newProjectNameBuffer = "";
            currentState = AppState::NewProjectPrompt;
        }
    }
    if (Button({210, 5, 80, 30}, "Load")) {
        std::string file = FileDialog::OpenFile("Project Files (*.qfx)\0*.qfx\0All Files (*.*)\0*.*\0", GetSaveDirectory().c_str());
        if (!file.empty()) {
            currentProject = Project();
            std::filesystem::path p(file);
            currentProject.projectDirectory = p.parent_path().string();
            currentProject.Load(file);
            currentProject.projectName = p.stem().string();
            currentState = AppState::Editing;
        }
    }
    if (Button({300, 5, 80, 30}, "Save")) {
        currentProject.Save(currentProject.projectDirectory + "/" + currentProject.projectName + ".qfx");
    }
    if (Button({390, 5, 100, 30}, "Save As")) {
        std::string file = FileDialog::SaveFile("Project Files (*.qfx)\0*.qfx\0All Files (*.*)\0*.*\0", GetSaveDirectory().c_str(), currentProject.projectName.c_str());
        if (!file.empty()) {
            std::filesystem::path p(file);
            currentProject.projectName = p.stem().string();
            currentProject.projectDirectory = p.parent_path().string();
            currentProject.Save(file);
        }
    }
    if (Button({500, 5, 160, 30}, "Open Location")) {
        std::string cmd = "explorer \"" + currentProject.projectDirectory + "\"";
        system(cmd.c_str());
    }
    if (Button({670, 5, 80, 30}, "Import")) {
        showImportSubMenu = !showImportSubMenu;
    }
    if (Button({760, 5, 80, 30}, "Export")) {
        std::string defaultName = currentProject.projectName + ".mp4";
        std::string file = FileDialog::SaveFile("MP4 Video (*.mp4)\0*.mp4\0", currentProject.projectDirectory.c_str(), defaultName.c_str());
        if (!file.empty()) {
            exporter.StartExport(file);
        }
    }
    if (Button({850, 5, 90, 30}, "Options")) {
    }
    if (Button({950, 5, 80, 30}, "Exit")) {
        if (currentProject.hasUnsavedChanges) {
            currentState = AppState::PromptSaveBeforeExit;
        } else {
            currentState = AppState::Exit;
        }
    }
    
    if (showImportSubMenu) {
        DrawRectangle(670, 35, 140, 80, LIGHTGRAY);
        DrawRectangleLines(670, 35, 140, 80, DARKGRAY);
        if (Button({675, 40, 130, 30}, "Add Sprite")) {
            showImportSubMenu = false;
            std::string desktopPath;
            if (const char* env_p = std::getenv("USERPROFILE")) {
                desktopPath = std::string(env_p) + "/Desktop";
            }
            std::vector<std::string> files = FileDialog::OpenFiles("Image Files (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0All Files (*.*)\0*.*\0", desktopPath.c_str());
            for (const auto& f : files) {
                std::filesystem::path p(f);
                std::filesystem::copy(f, currentProject.projectDirectory + "/sprite/" + p.filename().string(), std::filesystem::copy_options::overwrite_existing);
            }
        }
        if (Button({675, 75, 130, 30}, "Add Scene")) {
            showImportSubMenu = false;
            std::string desktopPath;
            if (const char* env_p = std::getenv("USERPROFILE")) {
                desktopPath = std::string(env_p) + "/Desktop";
            }
            std::vector<std::string> files = FileDialog::OpenFiles("Image Files (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0All Files (*.*)\0*.*\0", desktopPath.c_str());
            for (const auto& f : files) {
                std::filesystem::path p(f);
                std::string destPath = currentProject.projectDirectory + "/scene/" + p.filename().string();
                std::filesystem::copy(f, destPath, std::filesystem::copy_options::overwrite_existing);
                
                auto sceneEffect = std::make_shared<SceneEffect>(p.filename().string());
                currentProject.AddEffect(sceneEffect);
                sceneEffect->layer = -1;
                currentProject.SortEffectsByLayer();
            }
        }
    }
    
    DrawText(TextFormat("Project: %s%s", currentProject.projectName.c_str(), currentProject.hasUnsavedChanges ? "*" : ""), 1050, 10, 20, RAYWHITE);
}

void UIManager::DrawSidebar(Project &currentProject)
{
    DrawRectangle(1920 - 480, 40, 480, 900, Fade(RAYWHITE, 0.95f));
    DrawRectangleLines(1920 - 480, 40, 480, 900, DARKGRAY);

    if (promptingForScriptName) {
        DrawText("Enter Custom Effect Name:", 1920 - 480 + 20, 60, 24, BLACK);
        DrawRectangle(1920 - 480 + 20, 100, 440, 40, LIGHTGRAY);
        DrawRectangleLines(1920 - 480 + 20, 100, 440, 40, DARKGRAY);
        DrawText(customScriptNameBuffer.c_str(), 1920 - 480 + 30, 110, 20, BLACK);
        
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                customScriptNameBuffer += (char)key;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !customScriptNameBuffer.empty()) {
            customScriptNameBuffer.pop_back();
        }
        
        if (Button({1920 - 480 + 20, 160, 200, 40}, "Create")) {
            std::string hContent = "#pragma once\n#include \"Effect.h\"\n\nclass " + customScriptNameBuffer + " : public Effect {\npublic:\n    " + customScriptNameBuffer + "();\n    void Update(float dt) override;\n    void Draw() override;\n};\n";
            std::string cppContent = "#include \"" + customScriptNameBuffer + ".h\"\n\n" + customScriptNameBuffer + "::" + customScriptNameBuffer + "() {\n    name = \"" + customScriptNameBuffer + "\";\n}\n\nvoid " + customScriptNameBuffer + "::Update(float dt) {\n    // TODO: Update logic\n}\n\nvoid " + customScriptNameBuffer + "::Draw() {\n    // TODO: Draw logic\n}\n";
            
            std::ofstream hFile("src/" + customScriptNameBuffer + ".h");
            hFile << hContent;
            hFile.close();
            
            std::ofstream cppFile("src/" + customScriptNameBuffer + ".cpp");
            cppFile << cppContent;
            cppFile.close();
            
            std::string cmd = "code src/" + customScriptNameBuffer + ".h src/" + customScriptNameBuffer + ".cpp";
            system(cmd.c_str());
            
            promptingForScriptName = false;
            customScriptNameBuffer = "";
        }
        if (Button({1920 - 480 + 240, 160, 200, 40}, "Cancel")) {
            promptingForScriptName = false;
            customScriptNameBuffer = "";
        }
        return;
    }

    if (sidebarState == SidebarState::Selection) {
        DrawText("Usable Effects", 1920 - 480 + 20, 60, 24, BLACK);
        DrawLine(1920 - 480 + 20, 90, 1920 - 20, 90, DARKGRAY);

        if (Button({1920 - 480 + 20, 100, 440, 40}, "+ Add Snow Effect"))
        {
            currentProject.AddEffect(std::make_shared<SnowEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 150, 440, 40}, "+ Add Fire Effect"))
        {
            currentProject.AddEffect(std::make_shared<FireEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 200, 440, 40}, "+ Add Spark Effect"))
        {
            currentProject.AddEffect(std::make_shared<SparkEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 250, 440, 40}, "+ Add Nebula Effect"))
        {
            currentProject.AddEffect(std::make_shared<NebulaEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 300, 440, 40}, "+ Add Black Hole Effect"))
        {
            currentProject.AddEffect(std::make_shared<BlackHoleEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 350, 440, 40}, "+ Add Rain Effect"))
        {
            currentProject.AddEffect(std::make_shared<RainEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 400, 440, 40}, "+ Add Lightning Effect"))
        {
            currentProject.AddEffect(std::make_shared<LightningEffect>());
            currentProject.selectedEffectIndex = currentProject.activeEffects.size() - 1;
            sidebarState = SidebarState::Properties;
        }
        if (Button({1920 - 480 + 20, 450, 440, 40}, "+ Add custom script"))
        {
            promptingForScriptName = true;
            customScriptNameBuffer = "";
        }
    }
    else if (sidebarState == SidebarState::Properties) {
        if (Button({1920 - 480 + 20, 60, 100, 30}, "< Back")) {
            sidebarState = SidebarState::Selection;
            currentProject.selectedEffectIndex = -1;
        }
        if (Button({1920 - 480 + 360, 60, 100, 30}, "Delete")) {
            if (currentProject.selectedEffectIndex >= 0 && currentProject.selectedEffectIndex < currentProject.activeEffects.size()) {
                currentProject.activeEffects.erase(currentProject.activeEffects.begin() + currentProject.selectedEffectIndex);
            }
            sidebarState = SidebarState::Selection;
            currentProject.selectedEffectIndex = -1;
            return;
        }

        DrawLine(1920 - 480 + 20, 100, 1920 - 20, 100, DARKGRAY);
        DrawText("Properties", 1920 - 480 + 20, 110, 24, BLACK);

        if (currentProject.selectedEffectIndex >= 0 && currentProject.selectedEffectIndex < currentProject.activeEffects.size())
        {
            auto effect = currentProject.activeEffects[currentProject.selectedEffectIndex];
            DrawText(TextFormat("Editing: %s", effect->name.c_str()), 1920 - 480 + 20, 150, 20, DARKBLUE);

        if (auto snow = std::dynamic_pointer_cast<SnowEffect>(effect))
        {
            snow->speed = Slider({1920 - 480 + 20, 310, 300, 20}, "Speed", snow->speed, 0.0f, 1000.0f);
            FloatBox({1920 - 480 + 340, 300, 100, 30}, &snow->speed, 1);

            snow->density = Slider({1920 - 480 + 20, 370, 300, 20}, "Density", snow->density, 10.0f, 5000.0f);
            FloatBox({1920 - 480 + 340, 360, 100, 30}, &snow->density, 2);

            snow->directionAngle = Slider({1920 - 480 + 20, 430, 300, 20}, "Angle", snow->directionAngle, -180.0f, 360.0f);
            FloatBox({1920 - 480 + 340, 420, 100, 30}, &snow->directionAngle, 3);

            float seedFloat = (float)snow->seed;
            seedFloat = Slider({1920 - 480 + 20, 490, 300, 20}, "Seed", seedFloat, 0.0f, 100.0f);
            if (activeTextBox != 4)
                snow->seed = (int)seedFloat; 
            IntBox({1920 - 480 + 340, 480, 100, 30}, &snow->seed, 4);

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

            if (Button({1920 - 480 + 20, 610, 440, 40}, "Apply Density & Seed"))
            {
                snow->SetSeed(snow->seed);
                snow->InitParticles();
            }

            DrawText(TextFormat("Active: %s", snow->isActive ? "Yes" : "No"), 1920 - 480 + 20, 670, 20, BLACK);
            if (Button({1920 - 480 + 200, 665, 80, 30}, "Toggle"))
                snow->isActive = !snow->isActive;
        }
        else if (effect->name == "Fire")
        {
            auto fire = std::static_pointer_cast<FireEffect>(effect);
            DrawText("Fire Settings", 1920 - 480 + 20, 260, 20, BLACK);
            
            fire->wind = Slider({1920 - 480 + 20, 320, 440, 20},
                                TextFormat("wind force: %.1f", fire->wind), fire->wind, -2500.0f, 2500.0f);
            
            fire->spawnRate = (int)Slider({1920 - 480 + 20, 380, 440, 20},
                                          TextFormat("spawn rate: %d", fire->spawnRate), (float)fire->spawnRate, 10.0f, 200.0f);
            
            fire->position.x = Slider({1920 - 480 + 20, 440, 440, 20},
                                TextFormat("position X: %.0f", fire->position.x), fire->position.x, 0.0f, 1920.0f);
            
            fire->position.y = Slider({1920 - 480 + 20, 500, 440, 20},
                                TextFormat("position Y: %.0f", fire->position.y), fire->position.y, 0.0f, 1080.0f);
            
            DrawText(TextFormat("active: %s", fire->isActive ? "yes" : "no"), 1920 - 480 + 20, 560, 20, BLACK);
            if (Button({1920 - 480 + 200, 555, 80, 30}, "toggle"))
                fire->isActive = !fire->isActive;
        }
        else if (effect->name == "Spark") {
            auto sparks = std::static_pointer_cast<SparkEffect>(effect);
            DrawText("spark settings", 1920 - 480 + 20, 260, 20, BLACK);

            sparks->spawnRate = (int)Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("spawn rate: %d", sparks->spawnRate), (float)sparks->spawnRate, 1.0f, 50.0f);
            
            sparks->gravity = Slider({1920 - 480 + 20, 380, 440, 20},
                                TextFormat("gravity: %.1f", sparks->gravity), sparks->gravity, 100.0f, 3000.0f);
            
            sparks->position.x = Slider({1920 - 480 + 20, 440, 440, 20},
                                TextFormat("position X: %.0f", sparks->position.x), sparks->position.x, 0.0f, 1920.0f);
            
            sparks->position.y = Slider({1920 - 480 + 20, 500, 440, 20},
                                TextFormat("position Y: %.0f", sparks->position.y), sparks->position.y, 0.0f, 1080.0f);
            
            DrawText(TextFormat("active: %s", sparks->isActive ? "yes" : "no"), 1920 - 480 + 20, 560, 20, BLACK);
            if (Button({1920 - 480 + 200, 555, 80, 30}, "toggle"))
                sparks->isActive = !sparks->isActive;
        }
        else if (effect->name == "Nebula") {
            auto nebula = std::static_pointer_cast<NebulaEffect>(effect);
            DrawText("nebula settings", 1920 - 480 + 20, 260, 20, BLACK);
            
            nebula->spawnRate = (int)Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("spawn rate: %d", nebula->spawnRate), (float)nebula->spawnRate, 1.0f, 100.0f);
            
            nebula->drift = Slider({1920 - 480 + 20, 380, 440, 20},
                                TextFormat("drift: %.1f", nebula->drift), nebula->drift, 0.0f, 300.0f);
            
            nebula->position.x = Slider({1920 - 480 + 20, 440, 440, 20},
                                TextFormat("position X: %.0f", nebula->position.x), nebula->position.x, 0.0f, 1920.0f);
            
            nebula->position.y = Slider({1920 - 480 + 20, 500, 440, 20},
                                TextFormat("position Y: %.0f", nebula->position.y), nebula->position.y, 0.0f, 1080.0f);
            
            DrawText(TextFormat("active: %s", nebula->isActive ? "yes" : "no"), 1920 - 480 + 20, 560, 20, BLACK);
            if (Button({1920 - 480 + 200, 555, 80, 30}, "toggle"))
                nebula->isActive = !nebula->isActive;
        }
        else if (effect->name == "Black Hole") {
            auto blackHole = std::static_pointer_cast<BlackHoleEffect>(effect);
            DrawText("black hole settings", 1920 - 480 + 20, 260, 20, BLACK);
            
            blackHole->spawnRate = (int)Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("spawn rate: %d", blackHole->spawnRate), (float)blackHole->spawnRate, 1.0f, 400.0f);
            
            blackHole->pull = Slider({1920 - 480 + 20, 380, 440, 20},
                                TextFormat("pull: %.0f", blackHole->pull), blackHole->pull, 100000.0f, 8000000.0f);
            
            blackHole->center.x = Slider({1920 - 480 + 20, 440, 440, 20},
                                TextFormat("position X: %.0f", blackHole->center.x), blackHole->center.x, 0.0f, 1920.0f);
            
            blackHole->center.y = Slider({1920 - 480 + 20, 500, 440, 20},
                                TextFormat("position Y: %.0f", blackHole->center.y), blackHole->center.y, 0.0f, 1080.0f);
            
            DrawText(TextFormat("active: %s", blackHole->isActive ? "yes" : "no"), 1920 - 480 + 20, 560, 20, BLACK);
            if (Button({1920 - 480 + 200, 555, 80, 30}, "toggle"))
                blackHole->isActive = !blackHole->isActive;
        }
        else if (effect->name == "Rain") {
            auto rain = std::static_pointer_cast<RainEffect>(effect);
            DrawText("rain settings", 1920 - 480 + 20, 260, 20, BLACK);

            rain->stormIntensity = Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("storm intensity: %.2f", rain->stormIntensity), rain->stormIntensity, 0.0f, 5.0f);
            
            rain->gustStrength = Slider({1920 - 480 + 20, 380, 440, 20},
                                          TextFormat("gust strength: %.2f", rain->gustStrength), rain->gustStrength, 0.0f, 3.0f);
            
            rain->baseWindX = Slider({1920 - 480 + 20, 440, 440, 20},
                                          TextFormat("base wind: %.0f", rain->baseWindX), rain->baseWindX, -800.0f, 800.0f);
            
            DrawText(TextFormat("active: %s", rain->isActive ? "yes" : "no"), 1920 - 480 + 20, 500, 20, BLACK);
            if (Button({1920 - 480 + 200, 495, 80, 30}, "toggle"))
                rain->isActive = !rain->isActive;
        }
        else if (effect->name == "Lightning") {
            auto lightning = std::static_pointer_cast<LightningEffect>(effect);
            DrawText("lightning settings", 1920 - 480 + 20, 260, 20, BLACK);

            lightning->minStrikeInterval = Slider({1920 - 480 + 20, 320, 440, 20},
                                          TextFormat("min strike interval: %.2f", lightning->minStrikeInterval), lightning->minStrikeInterval, 0.1f, 5.0f);
            
            lightning->maxStrikeInterval = Slider({1920 - 480 + 20, 380, 440, 20},
                                          TextFormat("max strike interval: %.2f", lightning->maxStrikeInterval), lightning->maxStrikeInterval, 0.5f, 15.0f);
            
            lightning->masterFlashStrength = Slider({1920 - 480 + 20, 440, 440, 20},
                                          TextFormat("flash strength: %.2f", lightning->masterFlashStrength), lightning->masterFlashStrength, 0.0f, 3.0f);
            
            DrawText(TextFormat("active: %s", lightning->isActive ? "yes" : "no"), 1920 - 480 + 20, 500, 20, BLACK);
            if (Button({1920 - 480 + 200, 495, 80, 30}, "toggle"))
                lightning->isActive = !lightning->isActive;
        }
        else
        {
            DrawText("No effect selected.", 1920 - 480 + 20, 260, 20, GRAY);
        }
    }
    }
}

void UIManager::DrawTimeline(Project &currentProject)
{
    float timelineHeight = 250.0f;
    float timelineY = 1080.0f - timelineHeight;
    float pixelsPerSecond = 50.0f;
    float layerHeight = 40.0f;
    
    DrawRectangle(0, timelineY, 1920, timelineHeight, Fade(GRAY, 0.95f));
    DrawRectangleLines(0, timelineY, 1920, timelineHeight, DARKGRAY);
    
    if (Button({20, timelineY + 10, 60, 30}, currentProject.isPlaying ? "Pause" : "Play")) {
        currentProject.isPlaying = !currentProject.isPlaying;
    }
    if (Button({90, timelineY + 10, 60, 30}, "Stop")) {
        currentProject.isPlaying = false;
        currentProject.currentTime = 0.0f;
    }
    
    float rulerY = timelineY + 50.0f;
    DrawRectangle(0, rulerY, 1920, 20, LIGHTGRAY);
    for (int i = 0; i < currentProject.totalDuration; i++) {
        float px = 200.0f + (i * pixelsPerSecond) - timelineScrollX;
        if (px > 200 && px < 1920) {
            DrawLine(px, rulerY, px, rulerY + 10, BLACK);
            DrawText(TextFormat("%d", i), px + 2, rulerY + 2, 10, BLACK);
        }
    }
    
    float scrubberX = 200.0f + (currentProject.currentTime * pixelsPerSecond) - timelineScrollX;
    if (scrubberX > 200 && scrubberX < 1920) {
        DrawLineEx({scrubberX, rulerY}, {scrubberX, 1080.0f}, 2.0f, RED);
        DrawTriangle({scrubberX - 5, rulerY}, {scrubberX + 5, rulerY}, {scrubberX, rulerY + 10}, RED);
    }
    
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && currentDragMode == DragMode::None && mousePos.y >= rulerY && mousePos.y <= rulerY + 20 && mousePos.x > 200) {
        currentProject.currentTime = (mousePos.x - 200.0f + timelineScrollX) / pixelsPerSecond;
        if (currentProject.currentTime < 0) currentProject.currentTime = 0;
        if (currentProject.currentTime > currentProject.totalDuration) currentProject.currentTime = currentProject.totalDuration;
    }

    float tracksY = rulerY + 20.0f;
    
    for (int i = 0; i < currentProject.activeEffects.size(); i++)
    {
        auto& effect = currentProject.activeEffects[i];
        
        float startX = 200.0f + (effect->startTime * pixelsPerSecond) - timelineScrollX;
        float endX = 200.0f + (effect->endTime * pixelsPerSecond) - timelineScrollX;
        float width = endX - startX;
        float currentLayerY = tracksY + ((effect->layer + 1) * layerHeight) - timelineScrollY; 
        
        Rectangle blockBounds = {startX, currentLayerY, width, layerHeight - 5};
        Rectangle leftHandle = {startX, currentLayerY, 10.0f, layerHeight - 5};
        Rectangle rightHandle = {endX - 10.0f, currentLayerY, 10.0f, layerHeight - 5};

        bool isSelected = (currentProject.selectedEffectIndex == i);
        Color blockColor = isSelected ? BLUE : DARKBLUE;
        if (effect->name.find("Scene") != std::string::npos) blockColor = isSelected ? ORANGE : MAROON;
        
        DrawRectangleRec(blockBounds, Fade(blockColor, 0.7f));
        DrawRectangleLinesEx(blockBounds, 2, blockColor);
        DrawText(effect->name.c_str(), startX + 15, currentLayerY + 10, 10, RAYWHITE);
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && currentDragMode == DragMode::None) {
            if (CheckCollisionPointRec(mousePos, leftHandle)) {
                draggingEffectIndex = i;
                currentDragMode = DragMode::LeftEdge;
                currentProject.selectedEffectIndex = i;
                sidebarState = SidebarState::Properties;
            } else if (CheckCollisionPointRec(mousePos, rightHandle)) {
                draggingEffectIndex = i;
                currentDragMode = DragMode::RightEdge;
                currentProject.selectedEffectIndex = i;
                sidebarState = SidebarState::Properties;
            } else if (CheckCollisionPointRec(mousePos, blockBounds)) {
                draggingEffectIndex = i;
                currentDragMode = DragMode::Body;
                dragOffsetX = mousePos.x - startX;
                dragOffsetY = mousePos.y - currentLayerY;
                currentProject.selectedEffectIndex = i;
                sidebarState = SidebarState::Properties;
            }
        }
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && draggingEffectIndex != -1) {
        auto& effect = currentProject.activeEffects[draggingEffectIndex];
        float mouseTime = (mousePos.x - 200.0f + timelineScrollX) / pixelsPerSecond;
        
        if (currentDragMode == DragMode::LeftEdge) {
            effect->startTime = mouseTime;
            if (effect->startTime < 0) effect->startTime = 0;
            if (effect->startTime >= effect->endTime - 0.1f) effect->startTime = effect->endTime - 0.1f;
            currentProject.hasUnsavedChanges = true;
            currentProject.RebuildState();
        } else if (currentDragMode == DragMode::RightEdge) {
            effect->endTime = mouseTime;
            if (effect->endTime > currentProject.totalDuration) effect->endTime = currentProject.totalDuration;
            if (effect->endTime <= effect->startTime + 0.1f) effect->endTime = effect->startTime + 0.1f;
            currentProject.hasUnsavedChanges = true;
            currentProject.RebuildState();
        } else if (currentDragMode == DragMode::Body) {
            float duration = effect->endTime - effect->startTime;
            float newStart = (mousePos.x - dragOffsetX - 200.0f + timelineScrollX) / pixelsPerSecond;
            if (newStart < 0) newStart = 0;
            if (newStart + duration > currentProject.totalDuration) newStart = currentProject.totalDuration - duration;
            effect->startTime = newStart;
            effect->endTime = newStart + duration;
            
            float projectedY = mousePos.y - dragOffsetY;
            int newLayer = (int)((projectedY - tracksY + timelineScrollY) / layerHeight) - 1;
            if (newLayer != effect->layer) {
                effect->layer = newLayer;
                currentProject.SortEffectsByLayer();
            }
            currentProject.hasUnsavedChanges = true;
            currentProject.RebuildState();
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        currentDragMode = DragMode::None;
        draggingEffectIndex = -1;
    }
    
    DrawRectangle(0, rulerY, 200, 1080 - rulerY, DARKGRAY);
    DrawLine(200, rulerY, 200, 1080, BLACK);
    DrawText("Layers", 10, rulerY + 5, 10, RAYWHITE);
}