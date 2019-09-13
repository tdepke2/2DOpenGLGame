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

using namespace std;

class Character {
    public:
    TextureRect body;
    glm::vec2 position, velocity;
    float direction;
    
    Character();
    void draw();
};

#endif