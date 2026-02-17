#include "game.h"

// ============================================================================
// UTILITY
// ============================================================================

bool	aabb_collision(Vector2f p1, float r1, Vector2f p2, float r2) {
	float dx = p1._x - p2._x;
	float dy = p1._y - p2._y;
	float dist = std::sqrt(dx * dx + dy * dy);
	return dist < (r1 + r2);
}

void	resolve_collision(Vector2f& p1, float r1, Vector2f& p2, float r2) {
	Vector2f diff = p1 - p2;
	float dist = diff.length();
	
	// Éviter la division par zéro
	if (dist == 0) {
		// Si les deux entités sont exactement au même endroit, les séparer arbitrairement
		p1._x += 0.1f;
		p1._y += 0.1f;
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
