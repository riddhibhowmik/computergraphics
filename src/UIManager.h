#pragma once
#include "raylib.h"
#include "Project.h"

class UIManager {
public:
    UIManager();
    void UpdateAndDraw(Project& currentProject);
    
private:
    void DrawToolbar();
    void DrawSidebar(Project& currentProject);
    void DrawTimeline(Project& currentProject);
    bool Button(Rectangle bounds, const char* text);
    
    float Slider(Rectangle bounds, const char* text, float value, float minValue, float maxValue);
    bool FloatBox(Rectangle bounds, float* value, int id);
    bool IntBox(Rectangle bounds, int* value, int id);
    
    int activeTextBox = -1;
    std::string textInputBuffer = "";
};
