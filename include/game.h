#pragma once

#include <raylib.h>
#include <dirent.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

// ============================================================================
// CONSTANTS & ENUMS
// ============================================================================

enum class GameState {
	MENU,
	RUNNING,
	GAME_OVER,
	PAUSE
};

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int TARGET_FPS = 60;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct Vector2f;
struct Player;
struct Entity;
struct Room;
struct Dungeon;
struct Game;

// ============================================================================
// STRUCTS
// ============================================================================

struct Vector2f {
	float	_x;
	float	_y;
	
	Vector2f() : _x(0), _y(0) {}
	Vector2f(float x, float y) : _x(x), _y(y) {}
	
	Vector2f operator+(const Vector2f& v) const {
		return Vector2f(_x + v._x, _y + v._y);
	}
	
	Vector2f operator-(const Vector2f& v) const {
		return Vector2f(_x - v._x, _y - v._y);
	}
	
	Vector2f operator*(float scalar) const {
		return Vector2f(_x * scalar, _y * scalar);
	}
	
	bool operator==(const Vector2f& v) const {
		return (_x == v._x && _y == v._y);
	}
	
	bool operator!=(const Vector2f& v) const {
		return !(*this == v);
	}

	float length() const {
		return std::sqrt(_x * _x + _y * _y);
	}

	Vector2f normalized() const {
		float len = length();
		if (len == 0) return Vector2f(0, 0);
		return Vector2f(_x / len, _y / len);
	}
};

struct Player {
	Vector2f				_pos;			// Position
	Vector2f				_vel;			// Vélocité
	Vector2f				_acc;			// Accélération
	float					_speed;			// Vitesse max
	float					_radius;		// Rayon (collision)
	float					_hp;			// Points de vie
	float					_max_hp;		// Points de vie max
	float					_dash_cooldown;	// Cooldown du dash
	bool					_is_dashing;	// En train de dash ?
	float					_dash_duration;	// Durée du dash en cours
	float					_dash_speed;	// Vitesse du dash
		
	Player();
	void reset();
	void update(float dt);
	void draw() const;
};

struct Entity {
	enum Type {
		SKELETON,
		VAMPIRE,
		PRIEST,
		UNKNOWN
	};

	Type					_type;
	Vector2f				_pos;
	Vector2f				_vel;
	float					_radius;
	float					_hp;
	float					_max_hp;
	bool					_alive;
	float					_speed;
	float					_dammage;
		
	Entity(Type t = UNKNOWN, const Vector2f& p = Vector2f(0, 0));
	void update(float dt, const Player& player, const Room& room);
	void draw() const;

};

struct Room {
	enum Tile {
		WALL = 0,
		FLOOR = 1,
		DOOR_N = 2,
		DOOR_S = 3,
		DOOR_E = 4,
		DOOR_O = 5
	};

	int					_width;
	int					_height;
	int					_tile_size;
	std::vector<int>	_tiles;
	int					_room_id;
	Vector2f			_world_offset;

	Room();
	Room(int w, int h, int tile_size);
	bool load_from_file(const std::string& filename, int tile_size);
	Tile get_tile(int x, int y) const;
	void set_tile(int x, int y, Tile t);
	bool in_bounds(int x, int y) const;
	Vector2f get_spawn() const;
	bool is_walkable(const Vector2f& pos, float radius) const;
	void draw() const;
};

struct Dungeon {
	std::vector<Room>		_rooms;
	std::vector<Vector2f>	_connections;
	int						_current_room;
	Vector2f				_camera_target;
	Vector2f				_camera_pos;
	float					_camera_transition_speed;
	bool					_transitioning;

	Dungeon();
	void init();
	void load_rooms(int count, int tile_size);
	void connect_rooms();
	void update(float dt);
	void change_room(int new_room_id, const Vector2f& player_spawn);
	Room& current_room();
	const Room& current_room() const;
	void draw() const;
};

struct Game {
	GameState				_state;
	GameState				_next_state;
	Player					_player;
	Dungeon					_dungeon;
	std::vector<Entity>		_enemies;
	float					_time_elapsed;
	int						_score;
	int						_wave;
		
	Game();
	void init();
	void update(float dt);
	void draw() const;
	void handle_input();
	void change_state(GameState new_state);
	void spawn_enemy(Entity::Type type, const Vector2f& pos);
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool aabb_collision(Vector2f p1, float r1, Vector2f p2, float r2);
void resolve_collision(Vector2f& p1, float r1, Vector2f& p2, float r2);
