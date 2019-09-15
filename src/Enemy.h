#ifndef _ENEMY_H
#define _ENEMY_H

#include "Character.h"

using namespace std;

class Enemy : public Character {
    public:
    Character* targetPtr;
    float speed;
    int lifespan, damage;
    
    Enemy();
    bool isTouchingCharacter(const Character* character) const;
    int applyDamage(int damage);
    int update();
};

#endif