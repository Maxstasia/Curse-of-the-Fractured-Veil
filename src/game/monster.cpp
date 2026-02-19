#include "game.h"

// ============================================================================
// ENTITY
// ============================================================================

Entity::Entity(Type t, const Vector2f& p) : _type(t), _pos(p), _vel(0, 0), _alive(true), _shoot_timer(0), _shoot_cooldown(0) {
	if (_type == SKELETON){
		_radius = 12.0f;
		_hp = 30.0f;
		_max_hp = _hp;
		_speed = 150.0f;
		_dammage = 10.0f;
		_shoot_cooldown = 0; // pas de projectile
	}
	else if (_type == VAMPIRE) {
		_radius = 8.0f;
		_hp = 20.0f;
		_max_hp = _hp;
		_speed = 280.0f;
		_dammage = 5.0f;
		_shoot_cooldown = 0; // pas de projectile
	}
	else if (_type == PRIEST) {
		_radius = 30.0f;
		_hp = 40.0f;
		_max_hp = _hp;
		_speed = 100.0f;
		_dammage = 20.0f;
		_shoot_cooldown = 2.0f; // tire toutes les 2 secondes
	}
	else {
		_radius = 12.0f;
		_hp = 30.0f;
		_max_hp = _hp;
		_speed = 150.0f;
		_dammage = 10.0f;
		_shoot_cooldown = 0;
	}
}

void	Entity::update(float dt, const Player& player, const Room& room, std::vector<Projectile>& projectiles) {
	if (!_alive)
		return;
	
	// Priest tire des projectiles vers le joueur
	if (_type == PRIEST && _shoot_cooldown > 0) {
		_shoot_timer += dt;
		if (_shoot_timer >= _shoot_cooldown) {
			_shoot_timer = 0;
			Vector2f dir = (player._pos - _pos).normalized();
			Vector2f proj_vel = dir * 250.0f;
			projectiles.emplace_back(_pos + dir * _radius, proj_vel, _dammage * 0.5f, 6.0f, false, 3.0f);
		}
	}
	
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
	
	// Barre de vie au-dessus de l'ennemi
	float bar_width = _radius * 2.0f;
	float bar_height = 4.0f;
	float bar_x = _pos._x - bar_width * 0.5f;
	float bar_y = _pos._y - _radius - 10.0f;
	float hp_ratio = _hp / _max_hp;
	DrawRectangle((int)bar_x, (int)bar_y, (int)bar_width, (int)bar_height, DARKGRAY);
	DrawRectangle((int)bar_x, (int)bar_y, (int)(bar_width * hp_ratio), (int)bar_height, 
		hp_ratio > 0.5f ? GREEN : (hp_ratio > 0.25f ? YELLOW : RED));
}
