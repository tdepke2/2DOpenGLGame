#include "Projectile.h"
#include <cmath>

using namespace std;

Projectile::Projectile() {
    lifespan = 0;
}

void Projectile::update(list<Enemy>& targets) {
    glm::vec2 bottomLeft(position.x - size.x * 0.5f, position.y - size.x * 0.5f);
    glm::vec2 topRight(position.x + size.x * 0.5f, position.y + size.x * 0.5f);
    for (Enemy& enemy : targets) {
        if (enemy.health > 0 && checkCollisionAABB(bottomLeft, topRight, enemy.position - enemy.getSize() * 0.5f, enemy.position + enemy.getSize() * 0.5f)) {
            if (sqrt(pow(position.x - enemy.position.x, 2.0f) + pow(position.y - enemy.position.y, 2.0f)) < 50.0f) {
                enemy.applyDamage(damage);
                health -= damage;
                if (health <= 0) {
                    lifespan = 0;
                    return;
                }
            }
        }
    }
    position.x += velocity * cos(rotation);
    position.y += velocity * sin(rotation);
    --lifespan;
}