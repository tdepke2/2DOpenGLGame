#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Enemy.h"
#include "TextureRect.h"
#include <list>

using namespace std;

class Projectile : public TextureRect {
    public:
    float velocity;
    int lifespan, health, damage;
    
    Projectile();
    int update(list<Enemy>& targets);
};

#endif