#include "TextureRect.h"
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <SOIL/SOIL.h>
#include <stdexcept>

using namespace std;

pair<GLint, glm::uvec2> loadTexture(const string& filename) {
    glEnable(GL_TEXTURE_2D);
    GLint textureHandle = SOIL_load_OGL_texture(filename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    if(textureHandle == 0) {
        throw runtime_error("\"" + filename + "\": Unable to open texture file.");
    }
    
    FILE* textureFile;    // Get image dimensions (man I wish SOIL had this capability!)
    errno_t err = fopen_s(&textureFile, filename.c_str(), "rb");    // Source: http://www.wischik.com/lu/programmer/get-image-size.html
    if (err != 0) {
        throw runtime_error("\"" + filename + "\": Unable to read texture in binary mode.");
    }
    unsigned char buffer[24];
    fread(buffer, 1, 24, textureFile);
    fclose(textureFile);
    glm::uvec2 size;
    if (buffer[0] == 0x89 && buffer[1] == 'P' && buffer[2] == 'N' && buffer[3] == 'G') {
        size.x = (buffer[16] << 24) + (buffer[17] << 16) + (buffer[18] << 8) + (buffer[19] << 0);
        size.y = (buffer[20] << 24) + (buffer[21] << 16) + (buffer[22] << 8) + (buffer[23] << 0);
    } else {
        throw runtime_error("\"" + filename + "\": Image not recognized as a PNG, cannot get dimensions.");
    }
    
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glDisable(GL_TEXTURE_2D);
    
    return pair<GLint, glm::uvec2>(textureHandle, size);
}

bool checkCollisionAABB(const glm::vec2& aBottomLeft, const glm::vec2& aTopRight, const glm::vec2& bBottomLeft, const glm::vec2& bTopRight) {
    return (aBottomLeft.x < bTopRight.x && aTopRight.x > bBottomLeft.x && aBottomLeft.y < bTopRight.y && aTopRight.y > bBottomLeft.y);
}

TextureRect::TextureRect() {
    texture.first = 0;
    texture.second = glm::uvec2(0, 0);
    color = glm::uvec4(255, 255, 255, 255);
    position = glm::vec2(0.0f, 0.0f);
    origin = glm::vec2(0.0f, 0.0f);
    size = glm::vec2(0.0f, 0.0f);
    rotation = 0.0f;
    texCoordBottomLeft = glm::vec2(0.0f, 0.0f);
    texCoordTopRight = glm::vec2(1.0f, 1.0f);
}

TextureRect::TextureRect(const string& filename, const glm::vec2& position, const glm::vec2& size) : TextureRect(loadTexture(filename), position, size) {}

TextureRect::TextureRect(const pair<GLint, glm::uvec2>& texture, const glm::vec2& position, const glm::vec2& size) {
    this->texture = texture;
    color = glm::uvec4(255, 255, 255, 255);
    this->position = position;
    origin = glm::vec2(0.0f, 0.0f);
    this->size = size;
    rotation = 0.0f;
}

void TextureRect::centerOrigin() {
    origin.x = size.x / 2.0f;
    origin.y = size.y / 2.0f;
}

void TextureRect::draw() const {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&rotateMtx[0][0]); {
            glm::mat4 originMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-origin.x, -origin.y, 0.0f));
            glMultMatrixf(&originMtx[0][0]); {
                
                glBindTexture(GL_TEXTURE_2D, texture.first);
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_TRIANGLES); {
                    glColor4ub(color.r, color.g, color.b, color.a);
                    glTexCoord2f(texCoordBottomLeft.x, texCoordBottomLeft.y); glVertex2f(0.0f, 0.0f);
                    glTexCoord2f(texCoordTopRight.x, texCoordBottomLeft.y); glVertex2f(size.x, 0.0f);
                    glTexCoord2f(texCoordBottomLeft.x, texCoordTopRight.y); glVertex2f(0.0f, size.y);
                    
                    glTexCoord2f(texCoordTopRight.x, texCoordBottomLeft.y); glVertex2f(size.x, 0.0f);
                    glTexCoord2f(texCoordTopRight.x, texCoordTopRight.y); glVertex2f(size.x, size.y);
                    glTexCoord2f(texCoordBottomLeft.x, texCoordTopRight.y); glVertex2f(0.0f, size.y);
                }; glEnd();
                glDisable(GL_TEXTURE_2D);
                
            }; glMultMatrixf(&(glm::inverse(originMtx))[0][0]);
        }; glMultMatrixf(&(glm::inverse(rotateMtx))[0][0]);
    }; glMultMatrixf(&(glm::inverse(positionMtx))[0][0]);
}