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
    vector<vector<GLint>> bodyAnimations, feetAnimations;
    glm::vec2 position, velocity;
    float rotation;
    
    Character();
    const glm::vec2& getSize() const;
    int getCurrentBody() const;
    int getCurrentFeet() const;
    void setSize(const glm::vec2& size);
    void setCurrentBody(int bodyNumber);
    void setCurrentFeet(int feetNumber);
    void update();
    void draw();
    
    private:
    glm::vec2 _size, _origin;
    unsigned int _currentBody, _currentBodyNumber, _currentFeet, _currentFeetNumber;
};

#endif