#include "rect.hpp"

Rect::Rect() : pos(0.0f, 0.0f), sz(50.0f, 50.0f) {}

Rect::Rect(float x, float y, float w, float h) : pos(x, y), sz(w, h) {}

bool Rect::contains(sf::Vector2f point) const {
    return point.x >= pos.x && point.x <= pos.x + sz.x &&
           point.y >= pos.y && point.y <= pos.y + sz.y;
}

sf::Vector2f Rect::closestPoint(sf::Vector2f point) const {
    float cx = point.x;
    float cy = point.y;
    
    if (cx < pos.x) cx = pos.x;
    else if (cx > pos.x + sz.x) cx = pos.x + sz.x;
    
    if (cy < pos.y) cy = pos.y;
    else if (cy > pos.y + sz.y) cy = pos.y + sz.y;
    
    return {cx, cy};
}

sf::Vector2f Rect::center() const {
    return {pos.x + sz.x * 0.5f, pos.y + sz.y * 0.5f};
}
