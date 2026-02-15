#include "game.h"

// ============================================================================
// PLAYER
// ============================================================================

Player::Player()
    : pos(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2),
      vel(0, 0),
      acc(0, 0),
      speed(300.0f),
      radius(15.0f),
      hp(100.0f),
      max_hp(hp),
      dash_cooldown(1.0f),
      is_dashing(false),
      dash_duration(0),
      dash_speed(50.0f) {}

void Player::reset() {
    pos = Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    vel = Vector2f(0, 0);
    acc = Vector2f(0, 0);
    hp = max_hp;
    is_dashing = false;
    dash_duration = 0;
}

void Player::update(float dt) {
    // Input
    Vector2f input(0, 0);
    if (IsKeyDown(KEY_W)) input.y -= 1;
    if (IsKeyDown(KEY_S)) input.y += 1;
    if (IsKeyDown(KEY_A)) input.x -= 1;
    if (IsKeyDown(KEY_D)) input.x += 1;
	if (IsKeyDown(KEY_UP)) input.y -= 1;
	if (IsKeyDown(KEY_DOWN)) input.y += 1;
	if (IsKeyDown(KEY_LEFT)) input.x -= 1;
	if (IsKeyDown(KEY_RIGHT)) input.x += 1;
    
    // Normaliser input
    if (input.length() > 0) {
        input = input.normalized();
    }
    
    // Mise à jour du dash
    if (dash_cooldown > 0) {
        dash_cooldown -= dt;
    }
    
    if (is_dashing) {
        dash_duration -= dt;
        if (dash_duration <= 0) {
            is_dashing = false;
        }
    } else {
        // Mouvement normal
        acc = input * speed;
        vel = acc * dt;
    }
    
    pos = pos + vel;
    
    // Garder le joueur dans l'écran
    if (pos.x - radius < 0) pos.x = radius;
    if (pos.x + radius > SCREEN_WIDTH) pos.x = SCREEN_WIDTH - radius;
    if (pos.y - radius < 0) pos.y = radius;
    if (pos.y + radius > SCREEN_HEIGHT) pos.y = SCREEN_HEIGHT - radius;
}

void Player::draw() const {
    Color player_color = is_dashing ? YELLOW : BLUE;
    DrawCircleV({pos.x, pos.y}, radius, player_color);
    
    // HUD simple
    DrawText(TextFormat("HP: %.0f/%.0f", hp, max_hp), 10, 10, 20, WHITE);
    DrawText(TextFormat("Dash CD: %.2f", dash_cooldown), 10, 35, 20, WHITE);
}

// ============================================================================
// ENTITY
// ============================================================================

Entity::Entity(Type t, const Vector2f& p) : type(t), pos(p), vel(0, 0), alive(true) {
	if (type == SKELETON){
		radius = 12.0f;
		hp = 30.0f;
		max_hp = hp;
		speed = 150.0f;
	}
	else if (type == VAMPIRE) {
		radius = 8.0f;
		hp = 20.0f;
		max_hp = hp;
		speed = 280.0f;
	}
	else if (type == PRIEST) {
		radius = 30.0f;
		hp = 40.0f;
		max_hp = hp;
		speed = 100.0f;
	}
	else {
		radius = 12.0f;
		hp = 30.0f;
		max_hp = hp;
		speed = 150.0f;
	}
}

void Entity::update(float dt, const Player& player) {
    if (!alive)
		return;
    
	// Se diriger vers le joueur seulement si pas déjà en collision
	// La fonction aabb_collision vérifie si les cercles se touchent
	if (aabb_collision(pos, radius, player.pos, player.radius)) {
		return; // Ne pas bouger si déjà en contact avec le joueur
	}
	
	Vector2f dir = (player.pos - pos).normalized();
	vel = dir * speed * dt;
	pos = pos + vel;
}

void Entity::draw() const {
    if (!alive) return;
    
    Color entity_color = WHITE;
	if (type == SKELETON) entity_color = GRAY;
    if (type == VAMPIRE) entity_color = RED;
    if (type == PRIEST) entity_color = GREEN;
    
    DrawCircleV({pos.x, pos.y}, radius, entity_color);
}

// ============================================================================
// GAME
// ============================================================================

Game::Game()
    : state(GameState::MENU),
      next_state(GameState::MENU),
      time_elapsed(0),
      score(0),
      wave(0) {}

void Game::init() {
	state = GameState::MENU;
	next_state = GameState::MENU;
    time_elapsed = 0;
    score = 0;
    wave = 0;
}

void Game::update(float dt) {
    if (state != GameState::RUNNING) return;
	
    time_elapsed += dt;
    
    // Update joueur
    player.update(dt);
    
    // Update ennemis
    for (auto& enemy : enemies) {
        if (enemy.alive) {
            enemy.update(dt, player);
            
            // Check collision avec le joueur
            if (aabb_collision(player.pos, player.radius, enemy.pos, enemy.radius)) {
                player.hp -= 10.0f * dt;  // 10 dégâts/sec au contact
                // Résoudre la collision (repousser le monstre et le joueur)
                resolve_collision(player.pos, player.radius, enemy.pos, enemy.radius);
            }
        }
    }
    
    // Gérer les collisions entre les monstres
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].alive) continue;
        
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (!enemies[j].alive) continue;
            
            // Vérifier et résoudre la collision entre deux monstres
            if (aabb_collision(enemies[i].pos, enemies[i].radius, 
                              enemies[j].pos, enemies[j].radius)) {
                resolve_collision(enemies[i].pos, enemies[i].radius,
                                enemies[j].pos, enemies[j].radius);
            }
        }
    }
    
    // Nettoyer les ennemis morts
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const Entity& e) { return !e.alive; }),
        enemies.end()
    );
    
    // Check si le joueur est mort
    if (player.hp <= 0) {
        change_state(GameState::GAME_OVER);
    }
    
    // Spawn ennemis au fil du temps (test)
    static float spawn_timer = 0;
    spawn_timer += dt;
    if (spawn_timer > 3.0f && enemies.size() < 10) {
        float x = GetRandomValue(50, SCREEN_WIDTH - 50);
        float y = GetRandomValue(50, SCREEN_HEIGHT - 50);
		int WhoSpawn = rand() % 3;
		if (WhoSpawn == 0) {
			spawn_enemy(Entity::SKELETON, Vector2f(x, y));
		} else if (WhoSpawn == 1) {
			spawn_enemy(Entity::VAMPIRE, Vector2f(x, y));
		} else {
			spawn_enemy(Entity::PRIEST, Vector2f(x, y));
		}
        spawn_timer = 0;
    }
}

void Game::draw() const {
    if (state == GameState::MENU) {
        DrawText("CURSE OF THE FRACTURED VEIL", SCREEN_WIDTH/4.07, SCREEN_HEIGHT/2 - 100, 40, WHITE);
        DrawText("Press SPACE to start", SCREEN_WIDTH/2.37, SCREEN_HEIGHT/2 + 50, 20, GRAY);
    } else if (state == GameState::RUNNING) {
        player.draw();
        for (const auto& enemy : enemies) {
            enemy.draw();
        }
        DrawText(TextFormat("Wave: %d | Time: %.1f", wave, time_elapsed), 10, 60, 20, WHITE);
    } else if (state == GameState::GAME_OVER) {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 40, RED);
        DrawText(TextFormat("Score: %d", score), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 20, WHITE);
        DrawText("Press R to restart", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 80, 20, GRAY);
    }
}

void Game::handle_input() {
    if (IsKeyPressed(KEY_SPACE)) {
        if (state == GameState::MENU) {
            change_state(GameState::RUNNING);
        } else if (state == GameState::RUNNING) {
            // Dash
            if (player.dash_cooldown <= 0) {
                player.is_dashing = true;
                player.dash_duration = 0.2f;
                player.dash_cooldown = 1.0f;
                player.vel = player.vel.normalized() * player.dash_speed;
            }
        }
    }
    
    if (IsKeyPressed(KEY_R) && state == GameState::GAME_OVER) {
        init();
    }
}

void Game::change_state(GameState new_state) {
    state = new_state;
}

void Game::spawn_enemy(Entity::Type type, const Vector2f& pos) {
    enemies.emplace_back(type, pos);
}

// ============================================================================
// UTILITY
// ============================================================================

bool aabb_collision(Vector2f p1, float r1, Vector2f p2, float r2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    return dist < (r1 + r2);
}

void resolve_collision(Vector2f& p1, float r1, Vector2f& p2, float r2) {
    Vector2f diff = p1 - p2;
    float dist = diff.length();
    
    // Éviter la division par zéro
    if (dist == 0) {
        // Si les deux entités sont exactement au même endroit, les séparer arbitrairement
        p1.x += 0.1f;
        p1.y += 0.1f;
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
