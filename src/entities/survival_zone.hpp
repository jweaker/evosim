#pragma once
#include "rect.hpp"

class SurvivalZone : public Rect {
public:
    SurvivalZone() : Rect(0, 0, 100, 100) {}
    SurvivalZone(float x, float y, float w, float h) : Rect(x, y, w, h) {}
};
