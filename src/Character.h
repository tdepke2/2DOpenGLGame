#ifndef _CHARCTER_H
#define _CHARCTER_H

#include <GLFW/glfw3.h>    // OpenGL, GLFW, and GLM libs.
#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Animation.h"
#include "TextureRect.h"
#include <string>
#include <vector>

using namespace std;

class Character {
    public:
    const vector<Animation>* bodyAnimationsPtr;
    const vector<Animation>* feetAnimationsPtr;
    glm::vec2 position, velocity;
    float rotation;
    int health, iFrames, iFramesMax, bodyDelayCounter, feetDelayCounter;
    
    Character();
    const glm::vec2& getHitbox() const;
    float getScale() const;
    int getBody() const;
    int getFeet() const;
    int getBodyFrameNumber() const;
    int getFeetFrameNumber() const;
    void setScale(float scale);
    void setBody(int bodyNumber);
    void setFeet(int feetNumber);
    void applyDamage(int damage);
    void update();
    void draw() const;
    
    private:
    glm::vec2 _bodySize, _feetSize, _bodyOrigin, _feetOrigin;
    float _scale;
    int _currentBody, _currentBodyNumber, _currentFeet, _currentFeetNumber;
};

#endif