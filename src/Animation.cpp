#include "Animation.h"
#include "TextureRect.h"
#include <stdexcept>

using namespace std;

Animation::Animation() {
    size = glm::vec2(0.0f, 0.0f);
    delay = 0;
}

Animation::Animation(const vector<GLint>& frames, const glm::vec2& size, int delay) {
    this->frames = frames;
    this->size = size;
    this->delay = delay;
}

Animation::Animation(const string& filename, int start, int stop, int delay) {
    string startString = to_string(start);
    size_t numberPos = filename.rfind(startString);
    if (numberPos == string::npos) {
        throw runtime_error("\"" + filename + "\": Unable to load animation set.");
    }
    string leftSide = filename.substr(0, numberPos);
    string rightSide = filename.substr(numberPos + startString.length());
    
    pair<GLint, glm::uvec2> texture = loadTexture(filename);
    frames.push_back(texture.first);
    size = texture.second;
    for (int i = start + 1; i <= stop; ++i) {
        frames.push_back(loadTexture(leftSide + to_string(i) + rightSide).first);
    }
    this->delay = delay;
}