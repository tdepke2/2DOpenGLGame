#ifndef TEXTURE_RECT_H
#define TEXTURE_RECT_H

#include <GLFW/glfw3.h>    // OpenGL, GLFW, and GLM libs.
#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

using namespace std;

GLint loadTexture(const string& filename);

class TextureRect {
    public:
    GLint texture;
    glm::vec2 position, origin, size;
    float rotation;
    
    TextureRect();
    TextureRect(const string& filename, const glm::vec2& position, const glm::vec2& size);
    TextureRect(GLint textureHandle, const glm::vec2& position, const glm::vec2& size);
    void centerOrigin();
    void draw();
};

#endif