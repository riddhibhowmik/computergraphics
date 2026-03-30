#include "raylib.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "particle vfx engine");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("testing", 380, 340, 30, GREEN);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}