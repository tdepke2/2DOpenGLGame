#ifndef CHARCTER_H
#define CHARCTER_H

#include <GLFW/glfw3.h>    // OpenGL, GLFW, and GLM libs.
#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TextureRect.h"
#include <string>
#include <vector>

using namespace std;

vector<GLint> loadAnimation(string filename, int start, int stop);

class Character {
    public:
    const vector<vector<GLint>>* bodyAnimationsPtr;
    const vector<vector<GLint>>* feetAnimationsPtr;
    glm::vec2 position, velocity;
    float rotation;
    int health;
    
    Character();
    const glm::vec2& getSize() const;
    unsigned int getBody() const;
    unsigned int getFeet() const;
    unsigned int getBodyNumber() const;
    unsigned int getFeetNumber() const;
    void setSize(const glm::vec2& size);
    void setBody(int bodyNumber);
    void setFeet(int feetNumber);
    void update();
    void draw() const;
    
    private:
    glm::vec2 _size, _origin;
    unsigned int _currentBody, _currentBodyNumber, _currentFeet, _currentFeetNumber;
};

#endif