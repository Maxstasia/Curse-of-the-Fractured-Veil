#include "game.h"

// ============================================================================
// GAME
// ============================================================================

Game::Game()
	:	_state(GameState::MENU),
		_next_state(GameState::MENU),
		_time_elapsed(0),
		_score(0),
		_wave(0) {}

int		Game::init() {
	_state = GameState::MENU;
	_next_state = GameState::MENU;
	_time_elapsed = 0;
	_score = 0;
	_wave = 0;
	_dungeon.init();
	// Calculer la taille des tuiles proportionnellement à la résolution
	float scale_factor = (float)SCREEN_WIDTH / REFERENCE_WIDTH;
	int tile_size = (int)(REFERENCE_TILE_SIZE * scale_factor);
	
	if (_dungeon.scan_room_files(tile_size) != 0)
		return -1;

	// Charger la première salle
	if (!_dungeon.load_next_room())
		return -1;

	_player.reset();
	_player._pos = _dungeon.current_room().get_spawn();
	_enemies.clear();
	return 0;
}

void	Game::update(float dt) {
	if (_state != GameState::RUNNING)
		return ;
	
	_time_elapsed += dt;
	_dungeon.update(dt);
	
	// Update joueur
	Vector2f prev_pos = _player._pos;
	_player.update(dt);
	
	// Check si le joueur est encore dans la salle ou a changé de salle
	if (!_dungeon.current_room().is_walkable(_player._pos, _player._radius)) {
		// Check si on traverse une porte
		Vector2f local_pos = _player._pos - _dungeon.current_room()._world_offset;
		int tx = (int)std::floor(local_pos._x / _dungeon.current_room()._tile_size);
		int ty = (int)std::floor(local_pos._y / _dungeon.current_room()._tile_size);
		Room::Tile tile = _dungeon.current_room().get_tile(tx, ty);
		
		if (tile == Room::DOOR_N || tile == Room::DOOR_S || tile == Room::DOOR_E || tile == Room::DOOR_O) {
			// Charger une nouvelle salle aléatoire (pondérée par la progression)
			Room::Tile exit_dir = tile;
			if (_dungeon.load_next_room()) {
				// Spawn à la porte opposée de la direction de sortie
				Room::Tile opposite;
				if (exit_dir == Room::DOOR_N) opposite = Room::DOOR_S;
				else if (exit_dir == Room::DOOR_S) opposite = Room::DOOR_N;
				else if (exit_dir == Room::DOOR_E) opposite = Room::DOOR_O;
				else opposite = Room::DOOR_E;

				Vector2f spawn = _dungeon.current_room().get_door_position(opposite);
				if (spawn._x >= 0 && spawn._y >= 0)
					_player._pos = spawn;
				else
					_player._pos = _dungeon.current_room().get_spawn();
				_enemies.clear();
			}
		} else {
			// Collision avec le mur, annuler le mouvement
			_player._pos = prev_pos;
		}
	}
	
	// Update ennemis
	for (auto& enemy : _enemies) {
		if (enemy._alive) {
			enemy.update(dt, _player, _dungeon.current_room());
			
			// Check collision avec le joueur
			if (aabb_collision(_player._pos, _player._radius, enemy._pos, enemy._radius)) {
				_player._hp -= enemy._dammage * dt;
				// Sauvegarder la position du joueur avant résolution
				Vector2f player_pos_before = _player._pos;
				// Résoudre la collision (repousser le monstre et le joueur)
				resolve_collision(_player._pos, _player._radius, enemy._pos, enemy._radius);
				// Vérifier que le joueur n'est pas dans un mur après la résolution
				if (!_dungeon.current_room().is_walkable(_player._pos, _player._radius)) {
					// Si le joueur est dans un mur, le remettre à sa position précédente
					_player._pos = player_pos_before;
					// Et pousser seulement l'ennemi dans la direction opposée
					Vector2f push_dir = (enemy._pos - _player._pos).normalized();
					enemy._pos = _player._pos + push_dir * (_player._radius + enemy._radius);
				}
			}
		}
	}
	
	// Gérer les collisions entre les monstres
	for (size_t i = 0; i < _enemies.size(); ++i) {
		if (!_enemies[i]._alive)
			continue;
		
		for (size_t j = i + 1; j < _enemies.size(); ++j) {
			if (!_enemies[j]._alive)
				continue;
			
			// Vérifier et résoudre la collision entre deux monstres
			if (aabb_collision(_enemies[i]._pos, _enemies[i]._radius,
								_enemies[j]._pos, _enemies[j]._radius)) {
				resolve_collision(_enemies[i]._pos, _enemies[i]._radius,
								_enemies[j]._pos, _enemies[j]._radius);
			}
		}
	}
	
	// Nettoyer les ennemis morts
	_enemies.erase(std::remove_if(_enemies.begin(), _enemies.end(), [](const Entity& e) { return !e._alive; }), _enemies.end());
	
	// Check si le joueur est mort
	if (_player._hp <= 0)
		change_state(GameState::GAME_OVER);
	
	// Spawn ennemis au fil du temps dans la salle actuelle
	/* static float spawn_timer = 0;
	spawn_timer += dt;
	if (spawn_timer > 3.0f && _enemies.size() < 10) {
		Vector2f spawn_pos = _dungeon.current_room().get_spawn();
		int WhoSpawn = rand() % 3;
		if (WhoSpawn == 0) {
			spawn_enemy(Entity::SKELETON, spawn_pos);
		} else if (WhoSpawn == 1) {
			spawn_enemy(Entity::VAMPIRE, spawn_pos);
		} else {
			spawn_enemy(Entity::PRIEST, spawn_pos);
		}
		spawn_timer = 0;
	} */
}

void	Game::draw() const {
	if (_state == GameState::MENU) {
		DrawText("CURSE OF THE FRACTURED VEIL", SCREEN_WIDTH/4.07, SCREEN_HEIGHT/2 - 100, 40, WHITE);
		DrawText("Press SPACE to start", SCREEN_WIDTH/2.37, SCREEN_HEIGHT/2 + 50, 20, GRAY);
	} else if (_state == GameState::RUNNING) {
		_dungeon.draw();
		_player.draw();
		for (const auto& enemy : _enemies) {
			enemy.draw();
		}
		DrawText(TextFormat("Room: %d | Wave: %d | Time: %.1f", _dungeon._rooms_visited, _wave, _time_elapsed), 10, 60, 20, WHITE);
	} else if (_state == GameState::GAME_OVER) {
		DrawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 40, RED);
		DrawText(TextFormat("Score: %d", _score), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 20, WHITE);
		DrawText("Press R to restart", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 80, 20, GRAY);
	}
}

void	Game::handle_input() {
	if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT_SHIFT)) {
		if (_state == GameState::MENU) {
			change_state(GameState::RUNNING);
		} else if (_state == GameState::RUNNING) {
			// Dash
			if (_player._dash_cooldown <= 0) {
				_player._is_dashing = true;
				_player._dash_duration = 0.2f;
				_player._dash_cooldown = 1.0f;
				_player._vel = _player._vel.normalized() * _player._dash_speed;
			}
		}
	}
	
	if (IsKeyPressed(KEY_R) && _state == GameState::GAME_OVER) {
		init();
	}
}

void	Game::change_state(GameState new_state) {
	_state = new_state;
}

void	Game::spawn_enemy(Entity::Type type, const Vector2f& pos) {
	_enemies.emplace_back(type, pos);
}
