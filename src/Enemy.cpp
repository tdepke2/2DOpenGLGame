#include "Enemy.h"
#include "TextureRect.h"
#include <cmath>

#ifndef PI
#define PI acos(-1.0f)
#endif

using namespace std;

Enemy::Enemy() {
    targetPtr = nullptr;
}

bool Enemy::isTouchingCharacter(const Character* character) const {
    if (checkCollisionAABB(position - getSize() * 0.5f, position + getSize() * 0.5f, character->position - character->getSize() * 0.5f, character->position + character->getSize() * 0.5f)) {
        return (sqrt(pow(position.x - character->position.x, 2.0f) + pow(position.y - character->position.y, 2.0f)) < 50.0f);
    }
    return false;
}

void Enemy::update() {
    Character::update();
    if (targetPtr != nullptr) {
        if (targetPtr->position.x - position.x != 0.0f) {
            rotation = atan((targetPtr->position.y - position.y) / (targetPtr->position.x - position.x)) + (targetPtr->position.x - position.x > 0.0f ? 0.0f : PI);
        }
        if (getBody() == 0) {
            if (isTouchingCharacter(targetPtr)) {
                setBody(1);
            } else {
                position.x += 3.0f * cos(rotation);
                position.y += 3.0f * sin(rotation);
            }
        } else if (getBodyNumber() == 0 && !isTouchingCharacter(targetPtr)) {
            setBody(0);
        }
    }
}