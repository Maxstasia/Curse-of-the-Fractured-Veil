#include "game.h"

// ============================================================================
// PLAYER
// ============================================================================

Player::Player()
	:	_weapons{Weapon(Weapon::SWORD), Weapon(Weapon::BOW)},
		_active_weapon(0),
		_attack_timer(0),
		_facing(1, 0),
		_is_attacking(false),
		_attack_anim_timer(0),
		_pos(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2),
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
	_attack_timer = 0;
	_is_attacking = false;
	_attack_anim_timer = 0;
	_active_weapon = 0;
	_weapons[0] = Weapon(Weapon::SWORD);
	_weapons[1] = Weapon(Weapon::BOW);
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
	
	// Direction visée (vers la souris)
	Vector2 mouse = GetMousePosition();
	Vector2f mouse_dir(mouse.x - _pos._x, mouse.y - _pos._y);
	if (mouse_dir.length() > 0)
		_facing = mouse_dir.normalized();
	
	// Cooldown d'attaque
	if (_attack_timer > 0)
		_attack_timer -= dt;
	
	// Animation d'attaque
	if (_is_attacking) {
		_attack_anim_timer -= dt;
		if (_attack_anim_timer <= 0)
			_is_attacking = false;
	}
}

void	Player::draw() const {
	Color player_color = _is_dashing ? YELLOW : BLUE;
	DrawCircleV({_pos._x, _pos._y}, _radius, player_color);
	
	// Visualisation attaque épée (arc de swing)
	if (_is_attacking && _weapons[_active_weapon]._type == Weapon::SWORD) {
		Vector2f sword_end = _pos + _facing * _weapons[_active_weapon]._range;
		DrawLineEx({_pos._x, _pos._y}, {sword_end._x, sword_end._y}, 3.0f, WHITE);
		DrawCircleV({sword_end._x, sword_end._y}, 10.0f, {255, 255, 255, 150});
		// Arc d'attaque
		float angle = std::atan2(_facing._y, _facing._x);
		float arc_start = angle - 1.05f; // ~60 degrés de chaque côté
		for (int i = 0; i < 8; ++i) {
			float a = arc_start + (2.1f * i / 7.0f);
			Vector2f p = _pos + Vector2f(std::cos(a), std::sin(a)) * _weapons[_active_weapon]._range;
			DrawCircleV({p._x, p._y}, 2.0f, {255, 255, 255, 100});
		}
	}
	
	// Indicateur de direction (visée)
	Vector2f indicator = _pos + _facing * (_radius + 10.0f);
	Color indicator_color = WHITE;
	if (_weapons[_active_weapon]._type == Weapon::BOW)
		indicator_color = SKYBLUE;
	else if (_weapons[_active_weapon]._type == Weapon::STAFF)
		indicator_color = PURPLE;
	DrawCircleV({indicator._x, indicator._y}, 4.0f, indicator_color);
	
	// HUD - Points de vie
	DrawText(TextFormat("HP: %.0f/%.0f", _hp, _max_hp), 10, 10, 20, WHITE);
	DrawText(TextFormat("Dash CD: %.2f", _dash_cooldown), 10, 35, 20, WHITE);
	
	// HUD - Armes équipées
	const char* weapon_names[] = {"Epee", "Arc", "Baton"};
	Color slot1_color = (_active_weapon == 0) ? GOLD : GRAY;
	Color slot2_color = (_active_weapon == 1) ? GOLD : GRAY;
	DrawRectangle(SCREEN_WIDTH - 270, 5, 260, 75, {0, 0, 0, 150});
	DrawRectangleLines(SCREEN_WIDTH - 270, 5, 260, 75, (_active_weapon == 0) ? GOLD : GRAY);
	DrawText(TextFormat("[1] %s (dmg:%.0f)", weapon_names[_weapons[0]._type], _weapons[0]._damage), 
		SCREEN_WIDTH - 260, 12, 18, slot1_color);
	DrawText(TextFormat("[2] %s (dmg:%.0f)", weapon_names[_weapons[1]._type], _weapons[1]._damage), 
		SCREEN_WIDTH - 260, 38, 18, slot2_color);
	if (_attack_timer > 0)
		DrawText(TextFormat("Recharge: %.1fs", _attack_timer), SCREEN_WIDTH - 260, 58, 14, RED);
	else
		DrawText("Pret! (Clic gauche)", SCREEN_WIDTH - 260, 58, 14, GREEN);
}

void	Player::attack(std::vector<Entity>& enemies, std::vector<Projectile>& projectiles) {
	if (_attack_timer > 0)
		return;
	
	Weapon& w = _weapons[_active_weapon];
	_attack_timer = w._cooldown;
	_is_attacking = true;
	_attack_anim_timer = 0.2f;
	
	if (w._type == Weapon::SWORD) {
		// Attaque mêlée : touche tous les ennemis dans un cône devant le joueur
		for (auto& enemy : enemies) {
			if (!enemy._alive) continue;
			Vector2f diff = enemy._pos - _pos;
			float dist = diff.length();
			if (dist <= w._range + enemy._radius) {
				// Vérifier si l'ennemi est dans le cône (~120 degrés)
				Vector2f dir = diff.normalized();
				float dot = _facing._x * dir._x + _facing._y * dir._y;
				if (dot > 0.3f) {
					enemy._hp -= w._damage;
					if (enemy._hp <= 0)
						enemy._alive = false;
				}
			}
		}
	} else if (w._type == Weapon::BOW) {
		// Tir de flèche (rapide, petit)
		Vector2f proj_vel = _facing * 600.0f;
		projectiles.emplace_back(_pos + _facing * _radius, proj_vel, w._damage, 5.0f, true, 2.0f);
	} else if (w._type == Weapon::STAFF) {
		// Tir magique (plus lent, plus gros)
		Vector2f proj_vel = _facing * 400.0f;
		projectiles.emplace_back(_pos + _facing * _radius, proj_vel, w._damage, 8.0f, true, 2.5f);
	}
}

void	Player::switch_weapon() {
	_active_weapon = (_active_weapon + 1) % 2;
}
