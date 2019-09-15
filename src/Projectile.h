#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "Enemy.h"
#include "TextureRect.h"
#include <list>

using namespace std;

class Projectile : public TextureRect {
    public:
    float velocity;
    int lifespan, health, damage;
    
    Projectile();
    void update(list<Enemy>& targets);
};

#endif