#include "TileMap.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

TileMap::TileMap() {
    color = glm::uvec4(255, 255, 255, 255);
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
    glm::ivec2 tileMapSize(_textureSize.x / _tileSize.x, _textureSize.y / _tileSize.y);
    glm::vec2 extraTileOverlap(0.5f / _textureSize.x, 0.5f / _textureSize.y);    // Half pixel of extra overlap to fix lines in map.
    
    float texBottomLeftX = static_cast<float>(data % tileMapSize.x) / tileMapSize.x + extraTileOverlap.x;
    float texBottomLeftY = static_cast<float>(data / tileMapSize.x) / tileMapSize.y + extraTileOverlap.y;
    float texTopLeftX = texBottomLeftX + 1.0f / tileMapSize.x - 2.0f * extraTileOverlap.x;
    float texTopLeftY = texBottomLeftY + 1.0f / tileMapSize.y - 2.0f * extraTileOverlap.y;
    
    _texVertices[baseIndex] = glm::vec2(texBottomLeftX, texBottomLeftY);
    _texVertices[baseIndex + 1] = glm::vec2(texTopLeftX, texBottomLeftY);
    _texVertices[baseIndex + 2] = glm::vec2(texTopLeftX, texTopLeftY);
    _texVertices[baseIndex + 3] = glm::vec2(texBottomLeftX, texTopLeftY);
}

vector<vector<glm::vec2>> TileMap::loadMap(const string& filename, GLint textureHandle, const glm::uvec2& textureSize, const glm::uvec2& tileSize) {
    ifstream loadFile(filename);
    if (!loadFile.is_open()) {
        throw runtime_error("\"" + filename + "\": Unable to open level file.");
    }
    _texture = textureHandle;
    _textureSize = textureSize;
    _tileSize = tileSize;
    
    float levelVersion;
    string line;
    int lineNumber = 0, numEntries = 0, currentY;
    vector<vector<glm::vec2>> positionsData(3);
    try {
        while (getline(loadFile, line)) {
            ++lineNumber;
            vector<string> data = _parseCSV(line);
            
            if (data.size() == 1 && data[0].length() == 0) {
                continue;
            } else if (numEntries == 0 && data.size() == 2 && data[0] == "version") {
                levelVersion = stof(data[1]);
                if (levelVersion != 1.0f) {
                    throw runtime_error("Invalid map version.");
                }
                ++numEntries;
            } else if (numEntries == 1 && data.size() == 2 && data[0] == "name") {
                levelName = data[1];
                ++numEntries;
            } else if (numEntries == 2 && data.size() == 3 && data[0] == "size") {    // Allocate size of the level.
                _reallocateMap(glm::uvec2(stoul(data[1]), stoul(data[2])));
                currentY = _mapSize.y - 1;
                ++numEntries;
            } else if (numEntries == 3 && data.size() == 1 && data[0] == "player:") {
                ++numEntries;
            } else if (numEntries == 4) {    // Parse data for the player.
                if (data.size() == 1 && data[0] == "end") {
                    if (positionsData[0].size() != 1) {
                        throw runtime_error("Expected 1 entry.");
                    }
                    ++numEntries;
                } else if (data.size() == 2) {
                    positionsData[0].push_back(glm::vec2(stof(data[0]), stof(data[1])));
                } else {
                    throw runtime_error("Unrecognized player data.");
                }
            } else if (numEntries == 5 && data.size() == 1 && data[0] == "spawners:") {
                ++numEntries;
            } else if (numEntries == 6) {    // Parse data for all spawners.
                if (data.size() == 1 && data[0] == "end") {
                    if (positionsData[1].size() == 0) {
                        throw runtime_error("Expected at least 1 entry.");
                    }
                    ++numEntries;
                } else if (data.size() == 2) {
                    positionsData[1].push_back(glm::vec2(stof(data[0]), stof(data[1])));
                } else {
                    throw runtime_error("Unrecognized spawner data.");
                }
            } else if (numEntries == 7 && data.size() == 1 && data[0] == "items:") {
                ++numEntries;
            } else if (numEntries == 8) {    // Parse data for all items.
                if (data.size() == 1 && data[0] == "end") {
                    if (positionsData[2].size() != 3) {
                        throw runtime_error("Expected 3 entries.");
                    }
                    ++numEntries;
                } else if (data.size() == 2) {
                    positionsData[2].push_back(glm::vec2(stof(data[0]), stof(data[1])));
                } else {
                    throw runtime_error("Unrecognized item data.");
                }
            } else if (numEntries == 9 && data.size() == 1 && data[0] == "tiles:") {
                ++numEntries;
            } else if (numEntries == 10) {    // Parse data for all tiles.
                if (data.size() == 1 && data[0] == "end") {
                    ++numEntries;
                } else if (data.size() == _mapSize.x && currentY >= 0) {
                    for (unsigned int x = 0; x < _mapSize.x; ++x) {
                        setTile(stoi(data[x]), x, currentY);
                    }
                    --currentY;
                } else {
                    throw runtime_error("Unrecognized tile data.");
                }
            } else {
                throw runtime_error("Invalid level data.");
            }
        }
    } catch (exception& ex) {
        throw runtime_error("\"" + filename + "\" at line " + to_string(lineNumber) + ": " + ex.what());
    }
    
    if (numEntries != 11) {
        throw runtime_error("\"" + filename + "\": Missing level data, end of file reached.");
    }
    loadFile.close();
    return positionsData;
}

void TileMap::loadFont(GLint textureHandle, const glm::uvec2& textureSize, const glm::uvec2& tileSize) {
    _texture = textureHandle;
    _textureSize = textureSize;
    _tileSize = tileSize;
    _deleteMap();
}

void TileMap::loadText(const string& text) {
    glm::uvec2 fieldSize(0, 1);
    unsigned int lineLength = 0;
    for (unsigned int i = 0; i < text.length(); ++i) {
        if (text[i] == '\n') {
            ++fieldSize.y;
            if (lineLength > fieldSize.x) {
                fieldSize.x = lineLength;
            }
            lineLength = 0;
        } else {
            ++lineLength;
        }
    }
    if (lineLength > fieldSize.x) {
        fieldSize.x = lineLength;
    }
    if (fieldSize.x > _mapSize.x || fieldSize.y > _mapSize.y) {
        _reallocateMap(fieldSize);
        clearText();
    }
    
    unsigned int x = 0, y = _mapSize.y - 1;
    for (unsigned int i = 0; i < text.length(); ++i) {
        if (text[i] == '\n') {
            x = 0;
            --y;
        } else {
            if (_mapData[y][x] != text[i] - ' ') {
                setTile(text[i] - ' ', x, y);
            }
            ++x;
        }
    }
}

void TileMap::clearText() {
    for (unsigned int y = 0; y < _mapSize.y; ++y) {
        for (unsigned int x = 0; x < _mapSize.x; ++x) {
            setTile(0, x, y);
        }
    }
}

void TileMap::draw() const {
    glm::mat4 positionMtx = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    glMultMatrixf(&positionMtx[0][0]); {
        
        glBindTexture(GL_TEXTURE_2D, _texture);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES); {
            glColor4ub(color.r, color.g, color.b, color.a);
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

vector<string> TileMap::_parseCSV(const string& str) {
    vector<string> values;
    if (str.length() > 0 && str[0] == '#') {
        values.push_back("");
        return values;
    }
    
    size_t index = 0;
    while (index < str.length() && str[index] != ',') {
        ++index;
    }
    
    vector<size_t> points;
    points.push_back(index);
    while (index < str.length()) {
        ++index;
        while (index < str.length() && str[index] != ',') {
            ++index;
        }
        points.push_back(index);
    }
    
    values.reserve(points.size() - 1);
    values.push_back(str.substr(0, points[0]));
    for (unsigned int i = 1; i < points.size(); ++i) {
        values.push_back(str.substr(points[i - 1] + 1, points[i] - points[i - 1] - 1));
    }
    return values;
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

void TileMap::_reallocateMap(const glm::uvec2& newSize) {
    _deleteMap();
    _posVertices.reserve(newSize.x * newSize.y * 4);
    _texVertices.resize(newSize.x * newSize.y * 4);
    _mapSize = newSize;
    _mapData = new int*[_mapSize.y];
    for (unsigned int y = 0; y < _mapSize.y; ++y) {
        _mapData[y] = new int[_mapSize.x];
        for (unsigned int x = 0; x < _mapSize.x; ++x) {
            float windowX = static_cast<float>(x * _tileSize.x);
            float windowY = static_cast<float>(y * _tileSize.y);
            _posVertices.push_back(glm::vec2(windowX, windowY));
            _posVertices.push_back(glm::vec2(windowX + _tileSize.x, windowY));
            _posVertices.push_back(glm::vec2(windowX + _tileSize.x, windowY + _tileSize.y));
            _posVertices.push_back(glm::vec2(windowX, windowY + _tileSize.y));
        }
    }
}

void TileMap::_makeGLCoord(int vertexIndex) const {
    glTexCoord2f(_texVertices[vertexIndex].x, _texVertices[vertexIndex].y);
    glVertex2f(_posVertices[vertexIndex].x, _posVertices[vertexIndex].y);
}