#include "game.h"

int main() {	
	Game game;
	if (game.init() != 0)
	{
		if (IsWindowReady())
			CloseWindow();
		return 0;
	}
	
	// Init Raylib
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Curse of the Fractured Veil");
	SetTargetFPS(TARGET_FPS);
	// Boucle principale
	while (!WindowShouldClose()) {
		float dt = GetFrameTime();
		
		game.handle_input();
		game.update(dt);
		
		BeginDrawing();
		ClearBackground({00, 00, 30, 255});
		game.draw();
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
