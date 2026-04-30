#pragma once
#include "raylib.h"
#include "Project.h"
#include "ExportManager.h"

enum class AppState {
    MainMenu,
    NewProjectPrompt,
    NameExistsPrompt,
    Editing,
    PromptSaveBeforeNew,
    PromptSaveBeforeExit,
    Exit
};

class UIManager {
public:
    UIManager();
    void UpdateAndDraw(Project& currentProject, ExportManager& exporter);
    
    AppState currentState;
    
private:
    void DrawMainMenu(Project& currentProject);
    void DrawNewProjectPrompt(Project& currentProject);
    void DrawNameExistsPrompt(Project& currentProject);
    void DrawPromptSaveBeforeNew(Project& currentProject);
    void DrawPromptSaveBeforeExit(Project& currentProject);
    
    void DrawToolbar(Project& currentProject, ExportManager& exporter);
    void DrawSidebar(Project& currentProject);
    void DrawTimeline(Project& currentProject);
    bool Button(Rectangle bounds, const char* text);
    
    enum class SidebarState {
        Selection,
        Properties
    };
    SidebarState sidebarState = SidebarState::Selection;
    
    bool promptingForScriptName = false;
    std::string customScriptNameBuffer = "";
    
    bool showImportSubMenu = false;
    
    std::string newProjectNameBuffer = "";
    std::string pendingProjectName = "";
    
    float Slider(Rectangle bounds, const char* text, float value, float minValue, float maxValue);
    bool FloatBox(Rectangle bounds, float* value, int id);
    bool IntBox(Rectangle bounds, int* value, int id);
    
    int activeTextBox = -1;
    std::string textInputBuffer = "";
    
    // Timeline dragging state
    int draggingEffectIndex = -1;
    enum class DragMode { None, Body, LeftEdge, RightEdge };
    DragMode currentDragMode = DragMode::None;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;
    float timelineScrollX = 0.0f;
    float timelineScrollY = 0.0f;
    
    std::string GetSaveDirectory();
    std::string GetUniqueProjectName(const std::string& baseName);
    bool ProjectExists(const std::string& name);
    void CreateNewProject(Project& currentProject, const std::string& name);
};
