#include "game.h"

// ============================================================================
// ROOM
// ============================================================================

Room::Room() : _width(0), _height(0), _tile_size(32), _room_id(-1), _world_offset(0, 0) {}

Room::Room(int w, int h, int tile_size) 
	: _width(w), _height(h), _tile_size(tile_size), _room_id(-1), _world_offset(0, 0) {
	_tiles.assign(w * h, WALL);
}

bool Room::load_from_file(const std::string& filename, int tile_size) {
	std::ifstream file(filename);
	if (!file.is_open())
		return false;

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		lines.push_back(line);
	}
	file.close();

	if (lines.empty())
		return false;

	_height = lines.size();
	_width = lines[0].length();
	_tile_size = tile_size;
	_tiles.assign(_width * _height, WALL);

	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < (int)lines[y].length(); ++x) {
			char c = lines[y][x];
			Tile t = WALL;
			if (c == '.')
				t = FLOOR;
			else if (c == 'N')
				t = DOOR_N;
			else if (c == 'S')
				t = DOOR_S;
			else if (c == 'E')
				t = DOOR_E;
			else if (c == 'O')
				t = DOOR_O;
			set_tile(x, y, t);
		}
	}

	return true;
}

Room::Tile Room::get_tile(int x, int y) const {
	if (!in_bounds(x, y))
		return WALL;
	return (Tile)_tiles[y * _width + x];
}

void Room::set_tile(int x, int y, Tile t) {
	if (!in_bounds(x, y))
		return;
	_tiles[y * _width + x] = (int)t;
}

bool Room::in_bounds(int x, int y) const {
	return x >= 0 && y >= 0 && x < _width && y < _height;
}

Vector2f Room::get_spawn() const {
	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < _width; ++x) {
			Tile t = get_tile(x, y);
			if (t == FLOOR || t == DOOR_N || t == DOOR_S || t == DOOR_E || t == DOOR_O) {
				return _world_offset + Vector2f((x + 0.5f) * _tile_size, (y + 0.5f) * _tile_size);
			}
		}
	}
	return _world_offset + Vector2f(_width * _tile_size * 0.5f, _height * _tile_size * 0.5f);
}

bool Room::is_walkable(const Vector2f& pos, float radius) const {
	Vector2f local_pos = pos - _world_offset;
	int left = (int)std::floor((local_pos._x - radius) / _tile_size);
	int right = (int)std::floor((local_pos._x + radius) / _tile_size);
	int top = (int)std::floor((local_pos._y - radius) / _tile_size);
	int bottom = (int)std::floor((local_pos._y + radius) / _tile_size);

	if (!in_bounds(left, top) || !in_bounds(right, bottom))
		return false;

	Tile tl = get_tile(left, top);
	Tile tr = get_tile(right, top);
	Tile bl = get_tile(left, bottom);
	Tile br = get_tile(right, bottom);

	auto is_passable = [](Tile t) { return t != WALL; };
	return is_passable(tl) && is_passable(tr) && is_passable(bl) && is_passable(br);
}

void Room::draw() const {
	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < _width; ++x) {
			Tile t = get_tile(x, y);
			Color color = {40, 40, 50, 255};
			if (t == WALL)
				color = {18, 18, 25, 255};
			else if (t == DOOR_N || t == DOOR_S || t == DOOR_E || t == DOOR_O)
				color = {100, 100, 200, 255};

			Vector2f pos = _world_offset + Vector2f(x * _tile_size, y * _tile_size);
			DrawRectangle((int)pos._x, (int)pos._y, _tile_size, _tile_size, color);
		}
	}
}

// ============================================================================
// DUNGEON
// ============================================================================

Dungeon::Dungeon() 
	: _current_room(0), _camera_target(0, 0), _camera_pos(0, 0), 
	  _camera_transition_speed(500.0f), _transitioning(false) {}

void Dungeon::init() {
	_rooms.clear();
	_current_room = 0;
	_transitioning = false;
}

void Dungeon::load_rooms(int count, int tile_size) {
	_rooms.clear();

	std::string rooms_dir = "assets/ rooms";
	std::vector<std::string> room_files;

	// Lire le répertoire des salles
	DIR* dir = opendir(rooms_dir.c_str());
	if (dir != nullptr) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
			if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".room") {
				room_files.push_back(rooms_dir + "/" + filename);
			}
		}
		closedir(dir);
	}

	if (room_files.empty()) {
		// Créer une salle vide si pas de fichier
		Room default_room(10, 8, tile_size);
		for (int y = 1; y < 7; ++y) {
			for (int x = 1; x < 9; ++x) {
				default_room.set_tile(x, y, Room::FLOOR);
			}
		}
		default_room._room_id = 0;
		default_room._world_offset = Vector2f(0, 0);
		_rooms.push_back(default_room);
		return;
	}

	// Charger les salles et les placer
	for (int i = 0; i < count && i < (int)room_files.size(); ++i) {
		Room room;
		if (room.load_from_file(room_files[i], tile_size)) {
			room._room_id = i;
			room._world_offset = Vector2f(0, 0);
			_rooms.push_back(room);
		}
	}
}

void Dungeon::connect_rooms() {
	// Créer des connexions entre les salles basées sur leurs portes
	for (size_t i = 0; i < _rooms.size(); ++i) {
		// Trouver les portes disponibles
		for (int y = 0; y < _rooms[i]._height; ++y) {
			for (int x = 0; x < _rooms[i]._width; ++x) {
				Room::Tile t = _rooms[i].get_tile(x, y);
				if (t != Room::DOOR_N && t != Room::DOOR_S && t != Room::DOOR_E && t != Room::DOOR_O)
					continue;

				// Connecter à une autre salle aléatoire
				int other_idx = GetRandomValue(0, _rooms.size() - 2);
				if (other_idx >= (int)i)
					other_idx++;
			}
		}
	}
}

void Dungeon::update(float dt) {
	if (_transitioning) {
		Vector2f diff = _camera_target - _camera_pos;
		if (diff.length() < 10.0f) {
			_camera_pos = _camera_target;
			_transitioning = false;
		} else {
			Vector2f dir = diff.normalized();
			_camera_pos = _camera_pos + (dir * _camera_transition_speed * dt);
		}
	}
}

void Dungeon::change_room(int new_room_id, const Vector2f& player_spawn) {
	if (new_room_id < 0 || new_room_id >= (int)_rooms.size())
		return;

	_current_room = new_room_id;
	_camera_target = Vector2f(_rooms[new_room_id]._width * _rooms[new_room_id]._tile_size * 0.5f,
							   _rooms[new_room_id]._height * _rooms[new_room_id]._tile_size * 0.5f);
	_transitioning = true;
}

Room& Dungeon::current_room() {
	if (_current_room < 0 || _current_room >= (int)_rooms.size())
		_current_room = 0;
	return _rooms[_current_room];
}

const Room& Dungeon::current_room() const {
	if (_current_room < 0 || _current_room >= (int)_rooms.size())
		return _rooms[0];
	return _rooms[_current_room];
}

void Dungeon::draw() const {
	_rooms[_current_room].draw();
}

// ============================================================================
// PLAYER
// ============================================================================

Player::Player()
	:	_pos(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2),
		_vel(0, 0),
		_acc(0, 0),
		_speed(300.0f),
		_radius(15.0f),
		_hp(100.0f),
		_max_hp(_hp),
		_dash_cooldown(1.0f),
		_is_dashing(false),
		_dash_duration(0),
		_dash_speed(50.0f) {}

void	Player::reset() {
	_pos = Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	_vel = Vector2f(0, 0);
	_acc = Vector2f(0, 0);
	_hp = _max_hp;
	_is_dashing = false;
	_dash_duration = 0;
}

void	Player::update(float dt) {
	// Input
	Vector2f input(0, 0);
	if (IsKeyDown(KEY_W)) input._y -= 1;
	if (IsKeyDown(KEY_S)) input._y += 1;
	if (IsKeyDown(KEY_A)) input._x -= 1;
	if (IsKeyDown(KEY_D)) input._x += 1;
	if (IsKeyDown(KEY_UP)) input._y -= 1;
	if (IsKeyDown(KEY_DOWN)) input._y += 1;
	if (IsKeyDown(KEY_LEFT)) input._x -= 1;
	if (IsKeyDown(KEY_RIGHT)) input._x += 1;
	
	// Normaliser input
	if (input.length() > 0) {
		input = input.normalized();
	}
	
	// Mise à jour du dash
	if (_dash_cooldown > 0) {
		_dash_cooldown -= dt;
	}
	
	if (_is_dashing) {
		_dash_duration -= dt;
		if (_dash_duration <= 0) {
			_is_dashing = false;
		}
	} else {
		// Mouvement normal
		_acc = input * _speed;
		_vel = _acc * dt;
	}
	
	_pos = _pos + _vel;
	
	// Garder le joueur dans l'écran
	if (_pos._x - _radius < 0)
		_pos._x = _radius;
	if (_pos._x + _radius > SCREEN_WIDTH)
		_pos._x = SCREEN_WIDTH - _radius;
	if (_pos._y - _radius < 0)
		_pos._y = _radius;
	if (_pos._y + _radius > SCREEN_HEIGHT)
		_pos._y = SCREEN_HEIGHT - _radius;
}

void	Player::draw() const {
	Color player_color = _is_dashing ? YELLOW : BLUE;
	DrawCircleV({_pos._x, _pos._y}, _radius, player_color);
	
	// HUD simple
	DrawText(TextFormat("HP: %.0f/%.0f", _hp, _max_hp), 10, 10, 20, WHITE);
	DrawText(TextFormat("Dash CD: %.2f", _dash_cooldown), 10, 35, 20, WHITE);
}

// ============================================================================
// ENTITY
// ============================================================================

Entity::Entity(Type t, const Vector2f& p) : _type(t), _pos(p), _vel(0, 0), _alive(true) {
	if (_type == SKELETON){
		_radius = 12.0f;
		_hp = 30.0f;
		_max_hp = _hp;
		_speed = 150.0f;
		_dammage = 10.0f;
	}
	else if (_type == VAMPIRE) {
		_radius = 8.0f;
		_hp = 20.0f;
		_max_hp = _hp;
		_speed = 280.0f;
		_dammage = 5.0f;
	}
	else if (_type == PRIEST) {
		_radius = 30.0f;
		_hp = 40.0f;
		_max_hp = _hp;
		_speed = 100.0f;
		_dammage = 20.0f;
	}
	else {
		_radius = 12.0f;
		_hp = 30.0f;
		_max_hp = _hp;
		_speed = 150.0f;
		_dammage = 10.0f;
	}
}

void	Entity::update(float dt, const Player& player, const Room& room) {
	if (!_alive)
		return;
	
	// Se diriger vers le joueur seulement si pas déjà en collision
	if (aabb_collision(_pos, _radius, player._pos, player._radius))
		return;
	
	Vector2f dir = (player._pos - _pos).normalized();
	Vector2f next_pos = _pos + (dir * _speed * dt);
	if (room.is_walkable(next_pos, _radius)) {
		_pos = next_pos;
	}
}

void	Entity::draw() const {
	if (!_alive)
		return;
	
	Color entity_color = WHITE;
	if (_type == SKELETON)
		entity_color = GRAY;
	if (_type == VAMPIRE)
		entity_color = RED;
	if (_type == PRIEST)
		entity_color = GREEN;
	
	DrawCircleV({_pos._x, _pos._y}, _radius, entity_color);
}

// ============================================================================
// GAME
// ============================================================================

Game::Game()
	:	_state(GameState::MENU),
		_next_state(GameState::MENU),
		_time_elapsed(0),
		_score(0),
		_wave(0) {}

void	Game::init() {
	_state = GameState::MENU;
	_next_state = GameState::MENU;
	_time_elapsed = 0;
	_score = 0;
	_wave = 0;
	_dungeon.init();
	_dungeon.load_rooms(5, 32);
	_dungeon.connect_rooms();
	_player.reset();
	_player._pos = _dungeon.current_room().get_spawn();
	_enemies.clear();
}

void	Game::update(float dt) {
	if (_state != GameState::RUNNING)
		return;
	
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
			// Trouver la prochaine salle
			int next_room = (GetRandomValue(0, _dungeon._rooms.size() - 2));
			if (next_room >= _dungeon._current_room)
				next_room++;
			_dungeon.change_room(next_room, _dungeon._rooms[next_room].get_spawn());
			_player._pos = _dungeon.current_room().get_spawn();
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
				// Résoudre la collision (repousser le monstre et le joueur)
				resolve_collision(_player._pos, _player._radius, enemy._pos, enemy._radius);
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
	static float spawn_timer = 0;
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
	}
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
		DrawText(TextFormat("Room: %d | Wave: %d | Time: %.1f", _dungeon._current_room, _wave, _time_elapsed), 10, 60, 20, WHITE);
	} else if (_state == GameState::GAME_OVER) {
		DrawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 40, RED);
		DrawText(TextFormat("Score: %d", _score), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 20, WHITE);
		DrawText("Press R to restart", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 80, 20, GRAY);
	}
}

void	Game::handle_input() {
	if (IsKeyPressed(KEY_SPACE)) {
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

// ============================================================================
// UTILITY
// ============================================================================

bool	aabb_collision(Vector2f p1, float r1, Vector2f p2, float r2) {
	float dx = p1._x - p2._x;
	float dy = p1._y - p2._y;
	float dist = std::sqrt(dx * dx + dy * dy);
	return dist < (r1 + r2);
}

void	resolve_collision(Vector2f& p1, float r1, Vector2f& p2, float r2) {
	Vector2f diff = p1 - p2;
	float dist = diff.length();
	
	// Éviter la division par zéro
	if (dist == 0) {
		// Si les deux entités sont exactement au même endroit, les séparer arbitrairement
		p1._x += 0.1f;
		p1._y += 0.1f;
		dist = 0.1414f; // sqrt(0.1^2 + 0.1^2)
		diff = p1 - p2;
	}
	
	// Calculer la distance de chevauchement
	float overlap = (r1 + r2) - dist;
	
	if (overlap > 0) {
		// Normaliser la direction et déplacer les deux entités
		Vector2f direction = diff.normalized();
		
		// Déplacer chaque entité de la moitié du chevauchement
		Vector2f correction = direction * (overlap * 0.5f);
		p1 = p1 + correction;
		p2 = p2 - correction;
	}
}
