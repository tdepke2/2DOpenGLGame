#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

using namespace std;

class Enemy : public Character {
    public:
    const Character* targetPtr;
    
    Enemy();
    bool isTouchingCharacter(const Character* character) const;
    void update();
};

#endif