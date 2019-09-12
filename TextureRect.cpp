#include "TextureRect.h"
#include <iostream>
#include <SOIL/SOIL.h>

using namespace std;

GLint loadTexture(const string& filename) {
    glEnable(GL_TEXTURE_2D);
    GLint textureHandle = SOIL_load_OGL_texture(filename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);
    if(textureHandle == 0) {
        cout << "Error: Unable to open texture file \"" << filename << "\"." << endl;
        exit(-1);
    }
    glBindTexture(GL_TEXTURE_2D, textureHandle);    // This is copied from WorldMap.h file from assignment 2. I have no idea whats going on here, but it works!
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glDisable(GL_TEXTURE_2D);
    return textureHandle;
}

TextureRect::TextureRect() {
    texture = 0;
    position = glm::vec2(0.0f, 0.0f);
    origin = glm::vec2(0.0f, 0.0f);
    size = glm::vec2(0.0f, 0.0f);
    rotation = 0.0f;
}

TextureRect::TextureRect(const string& filename, const glm::vec2& position, const glm::vec2& size) {
    TextureRect(loadTexture(filename), position, size);
}

TextureRect::TextureRect(GLint textureHandle, const glm::vec2& position, const glm::vec2& size) {
    texture = textureHandle;
    this->position = position;
    origin = glm::vec2(0.0f, 0.0f);
    this->size = size;
    rotation = 0.0f;
}

void TextureRect::centerOrigin() {
    origin.x = size.x / 2.0f;
    origin.y = size.y / 2.0f;
}

void TextureRect::draw() {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&rotateMtx[0][0]); {
            glm::mat4 originMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-origin.x, -origin.y, 0.0f));
            glMultMatrixf(&originMtx[0][0]); {
                
                glBindTexture(GL_TEXTURE_2D, texture);
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_TRIANGLES); {
                    glColor4f(1, 1, 1, 1);
                    glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
                    glTexCoord2f(1, 0); glVertex2f(size.x, 0.0f);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, size.y);
                    
                    glTexCoord2f(1, 0); glVertex2f(size.x, 0.0f);
                    glTexCoord2f(1, 1); glVertex2f(size.x, size.y);
                    glTexCoord2f(0, 1); glVertex2f(0.0f, size.y);
                }; glEnd();
                glDisable(GL_TEXTURE_2D);
                
            }; glMultMatrixf(&(glm::inverse(originMtx))[0][0]);
        }; glMultMatrixf(&(glm::inverse(rotateMtx))[0][0]);
    }; glMultMatrixf(&(glm::inverse(positionMtx))[0][0]);
}