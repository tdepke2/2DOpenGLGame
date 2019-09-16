#include "Character.h"
#include "TextureRect.h"
#include <iostream>
#include <stdexcept>

using namespace std;

Character::Character() {
    bodyAnimationsPtr = nullptr;
    feetAnimationsPtr = nullptr;
    health = 0;
    iFrames = 0;
    iFramesMax = 0;
    bodyDelayCounter = 0;
    feetDelayCounter = 0;
    _scale = 0.0f;
    _currentBody = 0;
    _currentBodyNumber = 0;
    _currentFeet = 0;
    _currentFeetNumber = 0;
}

const glm::vec2& Character::getHitbox() const {
    return _bodySize;
}

float Character::getScale() const {
    return _scale;
}

int Character::getBody() const {
    return _currentBody;
}

int Character::getFeet() const {
    return _currentFeet;
}

int Character::getBodyFrameNumber() const {
    return _currentBodyNumber * ((*bodyAnimationsPtr)[_currentBody].delay + 1) + bodyDelayCounter;
}

int Character::getFeetFrameNumber() const {
    return _currentFeetNumber * ((*feetAnimationsPtr)[_currentFeet].delay + 1) + feetDelayCounter;
}

void Character::setScale(float scale) {
    _scale = scale;
    _bodySize = (*bodyAnimationsPtr)[_currentBody].size * scale;
    _bodyOrigin = _bodySize / 2.0f;
    if (feetAnimationsPtr != nullptr) {
        _feetSize = (*feetAnimationsPtr)[_currentFeet].size * scale;
        _feetOrigin = _feetSize / 2.0f;
    }
}

void Character::setBody(int bodyNumber) {
    if (_currentBody != bodyNumber) {
        _bodySize = (*bodyAnimationsPtr)[bodyNumber].size * _scale;
        _bodyOrigin = _bodySize / 2.0f;
        _currentBody = bodyNumber;
        _currentBodyNumber = 0;
    }
}

void Character::setFeet(int feetNumber) {
    if (_currentFeet != feetNumber) {
        _feetSize = (*feetAnimationsPtr)[feetNumber].size * _scale;
        _feetOrigin = _feetSize / 2.0f;
        _currentFeet = feetNumber;
        _currentFeetNumber = 0;
    }
}

void Character::applyDamage(int damage) {
    health = max(health - damage, 0);
    iFrames = iFramesMax;
}

void Character::update() {
    if (bodyDelayCounter >= (*bodyAnimationsPtr)[_currentBody].delay) {
        ++_currentBodyNumber;
        if (_currentBodyNumber >= static_cast<int>((*bodyAnimationsPtr)[_currentBody].frames.size())) {
            _currentBodyNumber = 0;
        }
        bodyDelayCounter = 0;
    } else {
        ++bodyDelayCounter;
    }
    
    if (feetAnimationsPtr != nullptr) {
        if (feetDelayCounter >= (*feetAnimationsPtr)[_currentFeet].delay) {
            ++_currentFeetNumber;
            if (_currentFeetNumber >= static_cast<int>((*feetAnimationsPtr)[_currentFeet].frames.size())) {
                _currentFeetNumber = 0;
            }
            feetDelayCounter = 0;
        } else {
            ++feetDelayCounter;
        }
    }
    
    if (iFrames > 0) {
        --iFrames;
    }
}

void Character::draw() const {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&rotateMtx[0][0]); {
            
            if (feetAnimationsPtr != nullptr) {
                glm::mat4 originMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-_feetOrigin.x - 10.0f, -_feetOrigin.y, 0.0f));
                glMultMatrixf(&originMtx[0][0]); {
                    glBindTexture(GL_TEXTURE_2D, (*feetAnimationsPtr)[_currentFeet].frames[_currentFeetNumber]);
                    glEnable(GL_TEXTURE_2D);
                    glBegin(GL_TRIANGLES); {
                        glColor4f(1, 1, 1, 1);
                        glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
                        glTexCoord2f(1, 0); glVertex2f(_feetSize.x, 0.0f);
                        glTexCoord2f(0, 1); glVertex2f(0.0f, _feetSize.y);
                        
                        glTexCoord2f(1, 0); glVertex2f(_feetSize.x, 0.0f);
                        glTexCoord2f(1, 1); glVertex2f(_feetSize.x, _feetSize.y);
                        glTexCoord2f(0, 1); glVertex2f(0.0f, _feetSize.y);
                    }; glEnd();
                    glDisable(GL_TEXTURE_2D);
                }; glMultMatrixf(&(glm::inverse(originMtx))[0][0]);
            }
            
            glm::mat4 originMtx2 = glm::translate(glm::mat4(1.0f), glm::vec3(-_bodyOrigin.x, -_bodyOrigin.y, 0.0f));
            glMultMatrixf(&originMtx2[0][0]); {
                glBindTexture(GL_TEXTURE_2D, (*bodyAnimationsPtr)[_currentBody].frames[_currentBodyNumber]);
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_TRIANGLES); {
                    glColor4f(1, 1, 1, 1);
                    glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
                    glTexCoord2f(1, 0); glVertex2f(_bodySize.x, 0.0f);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, _bodySize.y);
                    
                    glTexCoord2f(1, 0); glVertex2f(_bodySize.x, 0.0f);
                    glTexCoord2f(1, 1); glVertex2f(_bodySize.x, _bodySize.y);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, _bodySize.y);
                }; glEnd();
                glDisable(GL_TEXTURE_2D);
            }; glMultMatrixf(&(glm::inverse(originMtx2))[0][0]);
            
        }; glMultMatrixf(&(glm::inverse(rotateMtx))[0][0]);
    }; glMultMatrixf(&(glm::inverse(positionMtx))[0][0]);
}