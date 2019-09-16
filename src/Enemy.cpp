#include "Enemy.h"
#include "TextureRect.h"
#include <cmath>

#ifndef PI
#define PI acos(-1.0f)
#endif

using namespace std;

Enemy::Enemy() {
    targetPtr = nullptr;
    lifespan = 1000;
}

bool Enemy::isTouchingCharacter(const Character* character) const {
    if (checkCollisionAABB(position - getHitbox() * 0.5f, position + getHitbox() * 0.5f, character->position - character->getHitbox() * 0.5f, character->position + character->getHitbox() * 0.5f)) {
        return (sqrt(pow(position.x - character->position.x, 2.0f) + pow(position.y - character->position.y, 2.0f)) < 50.0f);
    }
    return false;
}

int Enemy::applyDamage(int damage) {
    Character::applyDamage(damage);
    if (health <= 0) {
        setBody(2);
        rotation = 2.0f * PI * static_cast<float>(rand()) / RAND_MAX;
        return 2;
    }
    return 0;
}

int Enemy::update() {
    int damageDealt = 0;
    Character::update();
    if (getBody() != 2) {
        if (targetPtr != nullptr) {
            if (targetPtr->position.x - position.x != 0.0f) {
                rotation = atan((targetPtr->position.y - position.y) / (targetPtr->position.x - position.x)) + (targetPtr->position.x - position.x > 0.0f ? 0.0f : PI);
            }
            if (getBody() == 0) {
                if (isTouchingCharacter(targetPtr)) {
                    setBody(1);
                    if (targetPtr->iFrames <= 0) {
                        targetPtr->applyDamage(damage);
                        damageDealt = damage;
                    }
                } else {
                    position.x += speed * cos(rotation);
                    position.y += speed * sin(rotation);
                }
            } else if (getBodyFrameNumber() == 0) {
                if (isTouchingCharacter(targetPtr)) {
                    if (targetPtr->iFrames <= 0) {
                        targetPtr->applyDamage(damage);
                        damageDealt = damage;
                    }
                } else {
                    setBody(0);
                }
            }
        }
    } else {
        --lifespan;
    }
    return damageDealt;
}