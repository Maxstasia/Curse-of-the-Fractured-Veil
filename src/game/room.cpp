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
	if (!file.is_open()) {
		printf("ERROR : Failed to load room file: %s\n", filename.c_str());
		return false;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == ' ') {
			printf("ERROR : Failed to load room file: %s\n", filename.c_str());
			return false;
		}
		lines.push_back(line);
	}
	file.close();

	if (lines.empty()) {
		printf("ERROR : Failed to load room file: %s (empty file)\n", filename.c_str());
		return false;
	}

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

Vector2f Room::get_door_position(Tile door_type) const {
	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < _width; ++x) {
			if (get_tile(x, y) == door_type) {
				float px = _world_offset._x + (x + 0.5f) * _tile_size;
				float py = _world_offset._y + (y + 0.5f) * _tile_size;
				// Décaler légèrement vers l'intérieur pour éviter de re-trigger la transition
				if (door_type == DOOR_N) py += _tile_size;
				else if (door_type == DOOR_S) py -= _tile_size;
				else if (door_type == DOOR_E) px -= _tile_size;
				else if (door_type == DOOR_O) px += _tile_size;
				return Vector2f(px, py);
			}
		}
	}
	return Vector2f(-1, -1);
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
	: _rooms_visited(0), _tile_size(64), _camera_target(0, 0), _camera_pos(0, 0), 
	  _camera_transition_speed(500.0f), _transitioning(false) {}

void Dungeon::init() {
	_rooms_visited = 0;
	_transitioning = false;
	_used_files.clear();
	_easy_files.clear();
	_medium_files.clear();
	_hard_files.clear();
	_boss_files.clear();
}

int Dungeon::scan_room_files(int tile_size) {
	_tile_size = tile_size;
	_easy_files.clear();
	_medium_files.clear();
	_hard_files.clear();
	_boss_files.clear();
	_used_files.clear();

	std::string base_path = ROOM_PATH;
	std::string categories[] = {"easy", "medium", "hard", "boss"};
	std::vector<std::string>* file_lists[] = {&_easy_files, &_medium_files, &_hard_files, &_boss_files};

	for (int i = 0; i < categories->size(); ++i) {
		std::string dir_path = base_path + "/" + categories[i];
		DIR* dir = opendir(dir_path.c_str());
		if (!dir) {
			printf("WARNING: Could not open room directory: %s\n", dir_path.c_str());
			continue;
		}
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
			if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".room") {
				file_lists[i]->push_back(dir_path + "/" + filename);
				printf("DEBUG: Found %s room: %s\n", categories[i].c_str(), filename.c_str());
			}
		}
		closedir(dir);
	}

	int total = _easy_files.size() + _medium_files.size() + _hard_files.size() + _boss_files.size();
	if (total == 0) {
		printf("ERROR: No room files found in %s!\n", base_path.c_str());
		return -1;
	}
	printf("DEBUG: Found %d total room files (easy:%d, medium:%d, hard:%d, boss:%d)\n",
		total, (int)_easy_files.size(), (int)_medium_files.size(), 
		(int)_hard_files.size(), (int)_boss_files.size());
	return 0;
}

std::string Dungeon::pick_next_room_file() {
	// Filtrer les fichiers déjà utilisés pour chaque catégorie
	std::vector<std::string> avail[4];
	const std::vector<std::string>* pools[4] = {&_easy_files, &_medium_files, &_hard_files, &_boss_files};

	for (int cat = 0; cat < 4; ++cat) {
		for (const auto& f : *pools[cat]) {
			if (std::find(_used_files.begin(), _used_files.end(), f) == _used_files.end())
				avail[cat].push_back(f);
		}
	}

	// Probabilités pondérées selon la progression du joueur
	// Plus le joueur avance, plus les salles hard/boss deviennent probables
	float weights[4];
	weights[0] = avail[0].empty() ? 0.0f : std::max(1.0f, 50.0f - _rooms_visited * 5.0f);	// easy
	weights[1] = avail[1].empty() ? 0.0f : std::max(1.0f, 30.0f - _rooms_visited * 2.0f);	// medium
	weights[2] = avail[2].empty() ? 0.0f : 15.0f + _rooms_visited * 3.0f;					// hard
	weights[3] = avail[3].empty() ? 0.0f : 5.0f + _rooms_visited * 4.0f;					// boss

	float total = weights[0] + weights[1] + weights[2] + weights[3];

	if (total <= 0.0f) {
		// Toutes les salles ont été visitées, on reset la liste
		if (_used_files.empty()) {
			printf("ERROR: No room files available at all!\n");
			return "";
		}
		printf("DEBUG: All rooms visited, resetting used files list\n");
		_used_files.clear();
		return pick_next_room_file();
	}

	printf("DEBUG: Room weights [easy:%.0f medium:%.0f hard:%.0f boss:%.0f] (visited:%d)\n",
		weights[0], weights[1], weights[2], weights[3], _rooms_visited);

	// Tirage aléatoire pondéré
	float roll = (float)random_int(0, 10000) / 10000.0f * total;
	int chosen_cat = 3;
	float cumulative = 0.0f;
	for (int i = 0; i < 4; ++i) {
		cumulative += weights[i];
		if (roll < cumulative) {
			chosen_cat = i;
			break;
		}
	}

	const char* cat_names[] = {"easy", "medium", "hard", "boss"};
	printf("DEBUG: Picked category: %s\n", cat_names[chosen_cat]);

	// Choisir un fichier au hasard dans la catégorie sélectionnée
	int idx = random_int(0, (int)avail[chosen_cat].size() - 1);
	return avail[chosen_cat][idx];
}

bool Dungeon::load_next_room() {
	std::string file = pick_next_room_file();
	if (file.empty())
		return false;

	Room new_room;
	if (!new_room.load_from_file(file, _tile_size))
		return false;

	// Centrer la salle sur l'écran
	float room_width = new_room._width * _tile_size;
	float room_height = new_room._height * _tile_size;
	new_room._world_offset = Vector2f(
		(SCREEN_WIDTH - room_width) * 0.5f,
		(SCREEN_HEIGHT - room_height) * 0.5f
	);
	new_room._room_id = _rooms_visited;

	_used_files.push_back(file);
	_active_room = new_room;
	_rooms_visited++;

	printf("DEBUG: Loaded room %s (total visited: %d)\n", file.c_str(), _rooms_visited);
	return true;
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

Room& Dungeon::current_room() {
	return _active_room;
}

const Room& Dungeon::current_room() const {
	return _active_room;
}

void Dungeon::draw() const {
	_active_room.draw();
}
