#include "game.h"

Weapon::Weapon(Type t)
	: _type(t) {
	switch (t) {
		case SWORD:
			_damage = 25.0f;
			_range = 50.0f;
			_cooldown = 0.5f;
			break;
		case BOW:
			_damage = 15.0f;
			_range = 200.0f;
			_cooldown = 1.0f;
			break;
		case STAFF:
			_damage = 20.0f;
			_range = 100.0f;
			_cooldown = 0.8f;
			break;
		default:
			_damage = 10.0f;
			_range = 50.0f;
			_cooldown = 1.0f;
			break;
	}
}

// ============================================================================
// PROJECTILE
// ============================================================================

Projectile::Projectile(const Vector2f& pos, const Vector2f& vel, float damage, float radius, bool from_player, float lifetime)
	: _pos(pos), _vel(vel), _damage(damage), _radius(radius), _lifetime(lifetime), _alive(true), _from_player(from_player) {}

void	Projectile::update(float dt, const Room& room) {
	if (!_alive)
		return;
	
	_pos = _pos + _vel * dt;
	_lifetime -= dt;
	
	if (_lifetime <= 0)
		_alive = false;
	
	// Collision avec les murs
	if (!room.is_walkable(_pos, _radius))
		_alive = false;
}

void	Projectile::draw() const {
	if (!_alive)
		return;
	
	Color color = _from_player ? SKYBLUE : ORANGE;
	DrawCircleV({_pos._x, _pos._y}, _radius, color);
	
	// Traînée visuelle
	Vector2f trail = _pos - _vel.normalized() * (_radius * 2.0f);
	DrawLineEx({trail._x, trail._y}, {_pos._x, _pos._y}, _radius * 0.6f, 
		_from_player ? Color{135, 206, 235, 100} : Color{255, 165, 0, 100});
}
