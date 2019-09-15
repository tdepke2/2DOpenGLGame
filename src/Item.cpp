#include "Item.h"

using namespace std;

Item::Item() {
    drawLabel = false;
    lifespan = 0;
}

Item::Item(GLint textureHandle, const glm::vec2& position, const glm::vec2& size, Type type) : TextureRect(textureHandle, position, size) {
    centerOrigin();
    this->type = type;
    drawLabel = false;
    lifespan = 0;
    
    if (type == PISTOL) {
        texCoordBottomLeft = glm::vec2(0.0f, 2.0f / 3.0f);
        texCoordTopRight = glm::vec2(2.0f / 4, 1.0f);
    } else if (type == RIFLE) {
        texCoordBottomLeft = glm::vec2(2.0f / 4.0f, 2.0f / 3.0f);
        texCoordTopRight = glm::vec2(1.0f, 1.0f);
    } else if (type == SHOTGUN) {
        texCoordBottomLeft = glm::vec2(0.0f, 1.0f / 3.0f);
        texCoordTopRight = glm::vec2(2.0f / 4.0f, 2.0f / 3.0f);
    } else if (type == HEALTH) {
        texCoordBottomLeft = glm::vec2(2.0f / 4.0f, 1.0f / 3.0f);
        texCoordTopRight = glm::vec2(3.0f / 4.0f, 2.0f / 3.0f);
    } else if (type == AMMO) {
        texCoordBottomLeft = glm::vec2(3.0f / 4.0f, 1.0f / 3.0f);
        texCoordTopRight = glm::vec2(1.0f, 2.0f / 3.0f);
    }
}

void Item::update() {
    if (lifespan > 0) {
        --lifespan;
    }
    rotation += 0.1f;
}

void Item::draw() const {
    TextureRect::draw();
    if (drawLabel) {
        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x - origin.x - 30.0f, position.y - origin.y - 20.0f, 0.0f));
        glMultMatrixf(&transMtx[0][0]); {
            glm::mat4 scaleMtx = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 1.0f));
            glMultMatrixf(&scaleMtx[0][0]); {
                labelTextMap.draw();
            }; glMultMatrixf(&(glm::inverse(scaleMtx))[0][0]);
        }; glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    }
}