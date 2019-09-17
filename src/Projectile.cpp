#include "Projectile.h"
#include <cmath>

using namespace std;

Projectile::Projectile() {
    lifespan = 0;
}

int Projectile::update(list<Enemy>& targets) {
    int numPoints = 0;
    for (Enemy& enemy : targets) {
        if (enemy.health > 0 && enemy.iFrames <= 0 && checkCollisionAABBDistance(position, glm::vec2(size.x, size.x), enemy.position, enemy.getHitbox()) < 50.0f) {
            numPoints += enemy.applyDamage(damage);
            health -= damage;
            if (health <= 0) {
                lifespan = 0;
                return numPoints;
            }
        }
    }
    position.x += velocity * cos(rotation);
    position.y += velocity * sin(rotation);
    --lifespan;
    return numPoints;
}