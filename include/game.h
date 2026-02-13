#pragma once

#include <raylib.h>
#include <vector>
#include <cmath>
#include <algorithm>

// ============================================================================
// CONSTANTS & ENUMS
// ============================================================================

enum class GameState {
    MENU,
    RUNNING,
    GAME_OVER,
    PAUSE
};

const int SCREEN_WIDTH = 2560;
const int SCREEN_HEIGHT = 1440;
const int TARGET_FPS = 120;

// ============================================================================
// STRUCTS
// ============================================================================

struct Vector2f {
    float x, y;
    
    Vector2f() : x(0), y(0) {}
    Vector2f(float x, float y) : x(x), y(y) {}
    
    Vector2f operator+(const Vector2f& v) const {
        return Vector2f(x + v.x, y + v.y);
    }
    
    Vector2f operator-(const Vector2f& v) const {
        return Vector2f(x - v.x, y - v.y);
    }
    
    Vector2f operator*(float scalar) const {
        return Vector2f(x * scalar, y * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    Vector2f normalized() const {
        float len = length();
        if (len == 0) return Vector2f(0, 0);
        return Vector2f(x / len, y / len);
    }
};

struct Player {
    Vector2f pos;           // Position
    Vector2f vel;           // Vélocité
    Vector2f acc;           // Accélération
    float speed;            // Vitesse max
    float radius;           // Rayon (collision)
    float hp;               // Points de vie
    float max_hp;           // Points de vie max
    float dash_cooldown;    // Cooldown du dash
    bool is_dashing;        // En train de dash ?
    float dash_duration;    // Durée du dash en cours
    float dash_speed;       // Vitesse du dash
    
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
    
    Type type;
    Vector2f pos;
    Vector2f vel;
    float radius;
    float hp;
    float max_hp;
    bool alive;
    float speed;
    
    Entity(Type t = UNKNOWN, const Vector2f& p = Vector2f(0, 0));
    void update(float dt, const Player& player);
    void draw() const;
};

struct Game {
    GameState state;
    GameState next_state;
    Player player;
    std::vector<Entity> enemies;
    float time_elapsed;
    int score;
    int wave;
    
    Game();
    void init();
	void play();
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
