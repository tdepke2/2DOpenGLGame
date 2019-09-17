#include "Enemy.h"
#include "TextureRect.h"
#include <cmath>

#ifndef PI
#define PI acos(-1.0f)
#endif

using namespace std;

Enemy::Enemy() {
    targetPtr = nullptr;
    itemDropPtr = nullptr;
    lifespan = 1000;
}

Enemy::~Enemy() {
    delete itemDropPtr;
}

bool Enemy::isTouchingCharacter(const Character* character) const {
    return checkCollisionAABBDistance(position, getHitbox(), character->position, character->getHitbox()) < 50.0f;
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
        if (lifespan < 255) {
            color.a = lifespan;
        }
    }
    return damageDealt;
}