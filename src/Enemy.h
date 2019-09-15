#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

using namespace std;

class Enemy : public Character {
    public:
    const Character* targetPtr;
    float speed;
    int lifespan;
    
    Enemy();
    bool isTouchingCharacter(const Character* character) const;
    void applyDamage(int damage);
    void update();
};

#endif