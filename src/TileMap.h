#ifndef TILE_MAP_H
#define TILE_MAP_H

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

class TileMap {
    public:
    glm::vec2 position;
    string levelName;
    
    TileMap();
    ~TileMap();
    const glm::uvec2& getMapSize() const;
    int getTile(int x, int y) const;
    void setTile(int data, int x, int y);
    void loadMap(const string& filename, GLint textureHandle, const glm::uvec2& textureSize, const glm::uvec2& tileSize);
    void draw() const;
    
    private:
    GLint _texture;
    glm::uvec2 _textureSize, _tileSize;
    vector<glm::vec2> _posVertices, _texVertices;
    glm::uvec2 _mapSize;
    int** _mapData;
    
    static vector<string> _parseCSV(const string& str);    // Parse a line of a CSV file. Splits the line and returns as a vector of strings, there will always be at least one string in the vector. Comments must be on their own line and start with a "#".
    void _deleteMap();
    void _makeGLCoord(int vertexIndex) const;
};

#endif