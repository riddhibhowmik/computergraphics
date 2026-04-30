#include "ExportManager.h"
#include <iostream>

ExportManager::ExportManager() {
    isExporting = false;
    ffmpegPipe = nullptr;
    currentExportTime = 0.0f;
    exportTarget = LoadRenderTexture(1920, 1080);
}

ExportManager::~ExportManager() {
    StopExport();
    UnloadRenderTexture(exportTarget);
}

bool ExportManager::StartExport(const std::string& path) {
    if (isExporting) return false;
    
    outputPath = path;
    
    // Set up ffmpeg command to read raw RGB24 from stdin
    std::string cmd = "ffmpeg -y -f rawvideo -vcodec rawvideo -s 1920x1080 -pix_fmt rgb24 -r 60 -i - -c:v libx264 -preset fast -crf 23 -pix_fmt yuv420p \"" + outputPath + "\"";
    
    ffmpegPipe = _popen(cmd.c_str(), "wb");
    if (!ffmpegPipe) {
        std::cerr << "Failed to open FFmpeg pipe. Ensure ffmpeg is installed and in your PATH." << std::endl;
        return false;
    }
    
    isExporting = true;
    currentExportTime = 0.0f;
    return true;
}

void ExportManager::StopExport() {
    if (isExporting) {
        isExporting = false;
        if (ffmpegPipe) {
            _pclose(ffmpegPipe);
            ffmpegPipe = nullptr;
        }
    }
}

void ExportManager::UpdateAndRender(Project& currentProject) {
    if (!isExporting) return;
    
    float dt = 1.0f / 60.0f; // Fixed timestep for export
    
    // Force project to exactly this time
    currentProject.currentTime = currentExportTime;
    currentProject.isPlaying = false; // Override normal playback
    
    // Update effects manually to this exact dt
    for (auto& effect : currentProject.activeEffects) {
        if (effect->isActive && currentProject.currentTime >= effect->startTime && currentProject.currentTime <= effect->endTime) {
            effect->Update(dt);
        }
    }
    
    BeginTextureMode(exportTarget);
    ClearBackground(BLACK);
    currentProject.Draw();
    EndTextureMode();
    
    // Read pixels back to RAM
    Image img = LoadImageFromTexture(exportTarget.texture);
    
    // Raylib textures are upside down when read, and format might need converting
    ImageFlipVertical(&img); 
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    
    // Write to FFmpeg
    if (ffmpegPipe) {
        fwrite(img.data, 1, img.width * img.height * 3, ffmpegPipe);
    }
    
    UnloadImage(img);
    
    currentExportTime += dt;
    
    if (currentExportTime >= currentProject.totalDuration) {
        StopExport();
        // Restore project state slightly
        currentProject.currentTime = 0.0f;
    }
}