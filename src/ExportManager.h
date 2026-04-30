#pragma once
#include <string>
#include <stdio.h>
#include "raylib.h"
#include "Project.h"

class ExportManager {
public:
    bool isExporting;
    std::string outputPath;
    FILE* ffmpegPipe;
    RenderTexture2D exportTarget;
    float currentExportTime;

    ExportManager();
    ~ExportManager();

    bool StartExport(const std::string& path);
    void UpdateAndRender(Project& currentProject);
    void StopExport();
};
