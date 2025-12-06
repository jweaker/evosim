#pragma once
#include "rect.hpp"
#include <random>

class SpawnZone : public Rect {
public:
    SpawnZone() : Rect(0, 0, 100, 100) {}
    SpawnZone(float x, float y, float w, float h) : Rect(x, y, w, h) {}
    
    sf::Vector2f randomPoint(std::mt19937& rng) const;
};
