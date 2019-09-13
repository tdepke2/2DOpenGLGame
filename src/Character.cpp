#include "Character.h"
#include "TextureRect.h"
#include <iostream>
#include <stdexcept>

using namespace std;

vector<GLint> loadAnimation(string filename, int start, int stop) {
    string startString = to_string(start);
    size_t numberPos = filename.rfind(startString);
    if (numberPos == string::npos) {
        throw runtime_error("\"" + filename + "\": Unable to load animation set.");
    }
    string leftSide = filename.substr(0, numberPos);
    string rightSide = filename.substr(numberPos + startString.length());
    vector<GLint> animation;
    
    for (int i = start; i <= stop; ++i) {
        animation.push_back(loadTexture(leftSide + to_string(i) + rightSide));
    }
    return animation;
}

Character::Character() {
    _currentBody = 0;
    _currentBodyNumber = 0;
    _currentFeet = 0;
    _currentFeetNumber = 0;
}

const glm::vec2& Character::getSize() const {
    return _size;
}

int Character::getCurrentBody() const {
    return _currentBody;
}

int Character::getCurrentFeet() const {
    return _currentFeet;
}

void Character::setSize(const glm::vec2& size) {
    _size = size;
    _origin.x = size.x / 2.0f;
    _origin.y = size.y / 2.0f;
}

void Character::setCurrentBody(int bodyNumber) {
    if (_currentBody != bodyNumber) {
        _currentBody = bodyNumber;
        _currentBodyNumber = 0;
    }
}

void Character::setCurrentFeet(int feetNumber) {
    if (_currentFeet != feetNumber) {
        _currentFeet = feetNumber;
        _currentFeetNumber = 0;
    }
}

void Character::update() {
    ++_currentBodyNumber;
    if (_currentBodyNumber >= bodyAnimations[_currentBody].size()) {
        _currentBodyNumber = 0;
    }
}

void Character::draw() {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&rotateMtx[0][0]); {
            glm::mat4 originMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-_origin.x, -_origin.y, 0.0f));
            glMultMatrixf(&originMtx[0][0]); {
                
                glBindTexture(GL_TEXTURE_2D, bodyAnimations[_currentBody][_currentBodyNumber]);
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_TRIANGLES); {
                    glColor4f(1, 1, 1, 1);
                    glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
                    glTexCoord2f(1, 0); glVertex2f(_size.x, 0.0f);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, _size.y);
                    
                    glTexCoord2f(1, 0); glVertex2f(_size.x, 0.0f);
                    glTexCoord2f(1, 1); glVertex2f(_size.x, _size.y);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, _size.y);
                }; glEnd();
                glDisable(GL_TEXTURE_2D);
                
            }; glMultMatrixf(&(glm::inverse(originMtx))[0][0]);
        }; glMultMatrixf(&(glm::inverse(rotateMtx))[0][0]);
    }; glMultMatrixf(&(glm::inverse(positionMtx))[0][0]);
}