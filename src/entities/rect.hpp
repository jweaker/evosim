#pragma once
#include <SFML/System/Vector2.hpp>

class Rect {
protected:
    sf::Vector2f pos;
    sf::Vector2f sz;

public:
    Rect();
    Rect(float x, float y, float w, float h);
    virtual ~Rect() = default;

    sf::Vector2f position() const { return pos; }
    sf::Vector2f size() const { return sz; }
    
    void setPosition(float x, float y) { pos = {x, y}; }
    void setSize(float w, float h) { sz = {w, h}; }

    bool contains(sf::Vector2f point) const;
    sf::Vector2f closestPoint(sf::Vector2f point) const;
    sf::Vector2f center() const;
};
