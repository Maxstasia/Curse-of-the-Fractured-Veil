#include "game.h"

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
