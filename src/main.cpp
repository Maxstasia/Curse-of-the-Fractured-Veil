#include "game.h"

int main() {
    // Init Raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Curse of the Fractured Veil");
    SetTargetFPS(TARGET_FPS);
    
    Game game;
    game.init();
    
    // Boucle principale
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        game.handle_input();
        game.update(dt);
        
        BeginDrawing();
        ClearBackground({20, 20, 30, 255});
        game.draw();
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
