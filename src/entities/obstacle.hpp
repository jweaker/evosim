#pragma once
#include "rect.hpp"

class Obstacle : public Rect {
public:
    Obstacle() : Rect() {}
    Obstacle(float x, float y, float w, float h) : Rect(x, y, w, h) {}
};
