#include "TileMap.h"

using namespace std;

TileMap::TileMap() {
    position = glm::vec2(0.0f, 0.0f);
    _texture = 0;
    _textureSize = glm::uvec2(0, 0);
    _tileSize = glm::uvec2(0, 0);
    _mapSize = glm::uvec2(0, 0);
    _mapData = nullptr;
}

TileMap::~TileMap() {
    _deleteMap();
}

const glm::uvec2& TileMap::getMapSize() const {
    return _mapSize;
}

int TileMap::getTile(int x, int y) const {
    return _mapData[y][x];
}

void TileMap::setTile(int data, int x, int y) {
    _mapData[y][x] = data;
    unsigned int baseIndex = (y * _mapSize.x + x) * 4;
    float texX = static_cast<float>(data % (_textureSize.x / _tileSize.x)) / _mapSize.x;
    float texY = static_cast<float>(data / (_textureSize.y / _tileSize.y)) / _mapSize.y;
    _texVertices[baseIndex] = glm::vec2(texX, texY);
    _texVertices[baseIndex + 1] = glm::vec2(texX + 1.0f / (_textureSize.x / _tileSize.x), texY);
    _texVertices[baseIndex + 2] = glm::vec2(texX + 1.0f / (_textureSize.x / _tileSize.x), texY + 1.0f / (_textureSize.y / _tileSize.y));
    _texVertices[baseIndex + 3] = glm::vec2(texX, texY + 1.0f / (_textureSize.y / _tileSize.y));
}

void TileMap::loadMap(GLint textureHandle, const glm::uvec2& textureSize, const glm::uvec2& tileSize, const glm::uvec2& mapSize) {
    _texture = textureHandle;
    _textureSize = textureSize;
    _tileSize = tileSize;
    _deleteMap();
    
    _posVertices.reserve(mapSize.x * mapSize.y * 4);
    _texVertices.resize(mapSize.x * mapSize.y * 4);
    _mapSize = mapSize;
    _mapData = new int*[_mapSize.y];
    int tempCounter = 14;
    for (unsigned int y = 0; y < _mapSize.y; ++y) {
        _mapData[y] = new int[_mapSize.x];
        for (unsigned int x = 0; x < _mapSize.x; ++x) {
            float windowX = static_cast<float>(x * _tileSize.x);
            float windowY = static_cast<float>(y * _tileSize.y);
            _posVertices.push_back(glm::vec2(windowX, windowY));
            _posVertices.push_back(glm::vec2(windowX + _tileSize.x, windowY));
            _posVertices.push_back(glm::vec2(windowX + _tileSize.x, windowY + _tileSize.y));
            _posVertices.push_back(glm::vec2(windowX, windowY + _tileSize.y));
            setTile(tempCounter, x, y);
            //++tempCounter;
        }
    }
}

void TileMap::draw() {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        
        glBindTexture(GL_TEXTURE_2D, _texture);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES); {
            glColor4f(1, 1, 1, 1);
            int baseIndex = 0;
            for (unsigned int y = 0; y < _mapSize.y; ++y) {
                for (unsigned int x = 0; x < _mapSize.x; ++x) {
                    _makeGLCoord(baseIndex);
                    _makeGLCoord(baseIndex + 1);
                    _makeGLCoord(baseIndex + 3);
                    
                    _makeGLCoord(baseIndex + 1);
                    _makeGLCoord(baseIndex + 2);
                    _makeGLCoord(baseIndex + 3);
                    baseIndex += 4;
                }
            }
        }; glEnd();
        glDisable(GL_TEXTURE_2D);
        
    }; glMultMatrixf(&(glm::inverse(positionMtx))[0][0]);
}

void TileMap::_deleteMap() {
    for (unsigned int y = 0; y < _mapSize.y; ++y) {
        delete[] _mapData[y];
    }
    delete[] _mapData;
    _posVertices.clear();
    _texVertices.clear();
    _mapSize = glm::uvec2(0, 0);
    _mapData = nullptr;
}

void TileMap::_makeGLCoord(int vertexIndex) {
    glTexCoord2f(_texVertices[vertexIndex].x, _texVertices[vertexIndex].y);
    glVertex2f(_posVertices[vertexIndex].x, _posVertices[vertexIndex].y);
}