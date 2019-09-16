#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <GLFW/glfw3.h>    // OpenGL, GLFW, and GLM libs.
#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

using namespace std;

class Animation {
    public:
    vector<GLint> frames;
    glm::vec2 size;
    int delay;
    
    Animation();
    Animation(const vector<GLint>& frames, const glm::vec2& size, int delay);
    Animation(const string& filename, int start, int stop, int delay);
};

#endif