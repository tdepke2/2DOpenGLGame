#ifndef _ITEM_H
#define _ITEM_H

#include "TextureRect.h"
#include "TileMap.h"

using namespace std;

class Item : public TextureRect {
    public:
    enum Type : int {
        PISTOL = 0, RIFLE, SHOTGUN, HEALTH, AMMO
    } type;
    
    TileMap labelTextMap;
    bool drawLabel;
    int lifespan;
    
    Item();
    Item(const pair<GLint, glm::uvec2>& texture, const glm::vec2& position, const glm::vec2& size, Type type, int lifespan);
    void update();
    void draw() const;
};

#endif