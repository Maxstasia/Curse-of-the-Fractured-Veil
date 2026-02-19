#pragma once

#include <raylib.h>
#include <dirent.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <random>

// ============================================================================
// CONSTANTS & ENUMS
// ============================================================================

enum class GameState {
	MENU,
	RUNNING,
	GAME_OVER,
	PAUSE
};

const int REFERENCE_WIDTH = 1920;
const int REFERENCE_HEIGHT = 1080;
const int REFERENCE_TILE_SIZE = 64;
// const int SCREEN_WIDTH = 2560;
// const int SCREEN_HEIGHT = 1440;
// const int SCREEN_WIDTH = 1920;
// const int SCREEN_HEIGHT = 1080;
// const int SCREEN_WIDTH = 1280;
// const int SCREEN_HEIGHT = 720;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1020;
const int TARGET_FPS = 60;

const std::string ROOM_PATH = "rooms";

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct Vector2f;
struct Weapon;
struct Projectile;
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
	
	Vector2f	operator+(const Vector2f& v) const {
		return Vector2f(_x + v._x, _y + v._y);
	}
	
	Vector2f	operator-(const Vector2f& v) const {
		return Vector2f(_x - v._x, _y - v._y);
	}
	
	Vector2f	operator*(float scalar) const {
		return Vector2f(_x * scalar, _y * scalar);
	}
	
	bool		operator==(const Vector2f& v) const {
		return (_x == v._x && _y == v._y);
	}
	
	bool		operator!=(const Vector2f& v) const {
		return !(*this == v);
	}

	float		length() const {
		return std::sqrt(_x * _x + _y * _y);
	}

	Vector2f	normalized() const {
		float len = length();
		if (len == 0) return Vector2f(0, 0);
		return Vector2f(_x / len, _y / len);
	}
};

struct Weapon {
	enum Type {
		SWORD,
		BOW,
		STAFF
	};

	Type	_type;
	float	_damage;
	float	_range;
	float	_cooldown;

	Weapon(Type t = SWORD);
};

struct Projectile {
	Vector2f	_pos;
	Vector2f	_vel;
	float		_damage;
	float		_radius;
	float		_lifetime;
	bool		_alive;
	bool		_from_player;
	
	Projectile(const Vector2f& pos, const Vector2f& vel, float damage, float radius, bool from_player, float lifetime = 3.0f);
	void		update(float dt, const Room& room);
	void		draw() const;
};

struct Player {
	Weapon					_weapons[2];		// 2 emplacements d'armes
	int						_active_weapon;		// Index arme active (0 ou 1)
	float					_attack_timer;		// Cooldown avant prochaine attaque
	Vector2f				_facing;			// Direction visée (vers la souris)
	bool					_is_attacking;		// Animation d'attaque en cours
	float					_attack_anim_timer;	// Timer animation attaque
	Vector2f				_pos;
	Vector2f				_vel;
	Vector2f				_acc;
	float					_speed;
	float					_radius;
	float					_hp;
	float					_max_hp;
	float					_dash_cooldown;
	bool					_is_dashing;
	float					_dash_duration;
	float					_dash_speed;
		
	Player();
	void		reset();
	void		update(float dt);
	void		draw() const;
	void		attack(std::vector<Entity>& enemies, std::vector<Projectile>& projectiles);
	void		switch_weapon();
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
	float					_shoot_timer;		// Timer tir projectile (Priest)
	float					_shoot_cooldown;	// Intervalle entre tirs
		
	Entity(Type t = UNKNOWN, const Vector2f& p = Vector2f(0, 0));
	void		update(float dt, const Player& player, const Room& room, std::vector<Projectile>& projectiles);
	void		draw() const;
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
	bool		load_from_file(const std::string& filename, int tile_size);
	Tile		get_tile(int x, int y) const;
	void		set_tile(int x, int y, Tile t);
	bool		in_bounds(int x, int y) const;
	Vector2f	get_spawn() const;
	Vector2f	get_door_position(Tile door_type) const;
	bool		is_walkable(const Vector2f& pos, float radius) const;
	void		draw() const;
};

struct Dungeon {
	Room						_active_room;
	int							_rooms_visited;
	int							_tile_size;
	Vector2f					_camera_target;
	Vector2f					_camera_pos;
	float						_camera_transition_speed;
	bool						_transitioning;

	// Pools de fichiers de salles par difficulté
	std::vector<std::string>	_easy_files;
	std::vector<std::string>	_medium_files;
	std::vector<std::string>	_hard_files;
	std::vector<std::string>	_boss_files;
	std::vector<std::string>	_used_files;

	Dungeon();
	void		init();
	int			scan_room_files(int tile_size);
	std::string	pick_next_room_file();
	bool		load_next_room();
	void		update(float dt);
	Room&		current_room();
	const Room&	current_room() const;
	void		draw() const;
};

struct Game {
	GameState				_state;
	GameState				_next_state;
	Player					_player;
	Dungeon					_dungeon;
	std::vector<Entity>		_enemies;
	std::vector<Projectile>	_projectiles;
	float					_time_elapsed;
	int						_score;
	int						_wave;
		
	Game();
	int			init();
	void		update(float dt);
	void		draw() const;
	void		handle_input();
	void		change_state(GameState new_state);
	void		spawn_enemy(Entity::Type type, const Vector2f& pos);
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool			aabb_collision(Vector2f p1, float r1, Vector2f p2, float r2);
void			resolve_collision(Vector2f& p1, float r1, Vector2f& p2, float r2);
int				random_int(int min, int max);
