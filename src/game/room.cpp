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

int Dungeon::load_rooms(int count, int tile_size) {
	_rooms.clear();

	std::string rooms_dir = ROOM_PATH;
	std::vector<std::string> room_files;

	// Lire le répertoire des salles
	DIR* dir = opendir(rooms_dir.c_str());
	if (dir != nullptr) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
			if (filename.length() > 4 && filename.substr(filename.length() - 5) == ".room") {
				room_files.push_back(rooms_dir + "/" + filename);
				printf("DEBUG : Found room file: %s\n", filename.c_str());
				printf("DEBUG : room_files : %s\n", room_files.back().c_str());
			}
		}
		closedir(dir);
	}
	else {
		printf("ERROR : Failed to open rooms directory: %s\n", rooms_dir.c_str());
		return -1;
	}

	if (room_files.empty()) {
		printf("ERROR : No room files found in directory: %s\n", rooms_dir.c_str());
		return -1;
	}

	// Charger les salles et les placer
	for (int i = 0; i < count && i < (int)room_files.size(); ++i) {
		Room room;
		if (room.load_from_file(room_files[i], tile_size)) {
			room._room_id = i;
			// Calculer l'offset pour centrer la salle sur l'écran
			float room_width = room._width * tile_size;
			float room_height = room._height * tile_size;
			float offset_x = (SCREEN_WIDTH - room_width) * 0.5f;
			float offset_y = (SCREEN_HEIGHT - room_height) * 0.5f;
			room._world_offset = Vector2f(offset_x, offset_y);
			_rooms.push_back(room);
		}
		else
			return -1;
	}
	return 0;
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
				int other_idx = rdm(0);
				printf("DEBUG : Connecting room %d to room %d\n", (int)i, other_idx);
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
	(void)player_spawn;
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
