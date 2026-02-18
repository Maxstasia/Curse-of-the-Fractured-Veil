#include "game.h"

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
