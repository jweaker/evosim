#include "widgets.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

Slider::Slider() 
    : minValue(0), maxValue(1), value(nullptr), isInteger(false), isLog(false), dragging(false) {}

Slider::Slider(float x, float y, float w, float h, float minV, float maxV,
               float* val, const std::string& lbl, bool isInt, bool log)
    : bounds({x, y}, {w, h}), minValue(minV), maxValue(maxV), value(val)
    , label(lbl), isInteger(isInt), isLog(log), dragging(false) {}

bool Slider::handleClick(sf::Vector2f pos) {
    if (bounds.contains(pos)) {
        dragging = true;
        handleDrag(pos);
        return true;
    }
    return false;
}

void Slider::handleDrag(sf::Vector2f pos) {
    if (!value) return;
    
    float norm = (pos.x - bounds.position.x) / bounds.size.x;
    norm = std::max(0.0f, std::min(1.0f, norm));
    
    if (isLog) {
        float logMin = std::log(minValue);
        float logMax = std::log(maxValue);
        *value = std::exp(logMin + norm * (logMax - logMin));
    } else {
        *value = minValue + norm * (maxValue - minValue);
    }
    
    if (isInteger) *value = std::round(*value);
}

void Slider::draw(sf::RenderWindow& window, const sf::Font& font) {
    sf::RectangleShape track({bounds.size.x, 6.0f});
    track.setPosition({bounds.position.x, bounds.position.y + bounds.size.y - 10.0f});
    track.setFillColor(sf::Color(40, 40, 45));
    track.setOutlineColor(sf::Color(60, 60, 65));
    track.setOutlineThickness(1);
    window.draw(track);
    
    float norm;
    if (isLog) {
        float logMin = std::log(minValue);
        float logMax = std::log(maxValue);
        norm = (std::log(*value) - logMin) / (logMax - logMin);
    } else {
        norm = (*value - minValue) / (maxValue - minValue);
    }
    norm = std::max(0.0f, std::min(1.0f, norm));
    
    sf::RectangleShape fill({bounds.size.x * norm, 6.0f});
    fill.setPosition({bounds.position.x, bounds.position.y + bounds.size.y - 10.0f});
    fill.setFillColor(sf::Color(70, 130, 180));
    window.draw(fill);
    
    float hx = bounds.position.x + norm * bounds.size.x;
    float hy = bounds.position.y + bounds.size.y - 7.0f;
    
    sf::CircleShape handle(7.0f);
    handle.setOrigin({7.0f, 7.0f});
    handle.setPosition({hx, hy});
    handle.setFillColor(dragging ? sf::Color(100, 160, 220) : sf::Color(85, 145, 200));
    handle.setOutlineColor(sf::Color(255, 255, 255, 180));
    handle.setOutlineThickness(2);
    window.draw(handle);
    
    sf::Text labelText(font, label, 14);
    labelText.setPosition({bounds.position.x, bounds.position.y});
    labelText.setFillColor(sf::Color(200, 200, 200));
    window.draw(labelText);
    
    std::ostringstream ss;
    if (isInteger) {
        ss << (int)*value;
    } else {
        ss << std::fixed << std::setprecision(2) << *value;
    }
    
    sf::Text valText(font, ss.str(), 14);
    sf::FloatRect vb = valText.getLocalBounds();
    valText.setPosition({bounds.position.x + bounds.size.x - vb.size.x, bounds.position.y});
    valText.setFillColor(sf::Color::White);
    window.draw(valText);
}

Toggle::Toggle() : value(nullptr) {}

Toggle::Toggle(float x, float y, float w, float h, bool* val, const std::string& lbl)
    : bounds({x, y}, {w, h}), value(val), label(lbl) {}

bool Toggle::handleClick(sf::Vector2f pos) {
    if (bounds.contains(pos) && value) {
        *value = !*value;
        return true;
    }
    return false;
}

void Toggle::draw(sf::RenderWindow& window, const sf::Font& font) {
    bool on = value && *value;
    
    float sw = 36.0f, sh = 18.0f;
    float sx = bounds.position.x + bounds.size.x - sw - 5;
    float sy = bounds.position.y + (bounds.size.y - sh) / 2;
    
    sf::RectangleShape bg({sw, sh});
    bg.setPosition({sx, sy});
    bg.setFillColor(on ? sf::Color(70, 150, 70) : sf::Color(60, 60, 65));
    bg.setOutlineColor(sf::Color(80, 80, 85));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    float kr = 7.0f;
    float kx = on ? (sx + sw - kr - 3) : (sx + kr + 3);
    
    sf::CircleShape knob(kr);
    knob.setOrigin({kr, kr});
    knob.setPosition({kx, sy + sh / 2});
    knob.setFillColor(sf::Color::White);
    window.draw(knob);
    
    sf::Text text(font, label, 14);
    text.setPosition({bounds.position.x, bounds.position.y + (bounds.size.y - 14) / 2});
    text.setFillColor(sf::Color(200, 200, 200));
    window.draw(text);
}

Button::Button() {}

Button::Button(float x, float y, float w, float h, const std::string& lbl)
    : bounds({x, y}, {w, h}), label(lbl) {}

bool Button::handleClick(sf::Vector2f pos) {
    return bounds.contains(pos);
}

void Button::draw(sf::RenderWindow& window, const sf::Font& font, bool highlighted) {
    sf::RectangleShape bg(bounds.size);
    bg.setPosition(bounds.position);
    bg.setFillColor(highlighted ? sf::Color(80, 140, 180) : sf::Color(50, 52, 58));
    bg.setOutlineColor(sf::Color(70, 72, 78));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    sf::Text text(font, label, 12);
    sf::FloatRect tb = text.getLocalBounds();
    text.setPosition({
        bounds.position.x + (bounds.size.x - tb.size.x) / 2,
        bounds.position.y + (bounds.size.y - tb.size.y) / 2 - 2
    });
    text.setFillColor(sf::Color(220, 220, 225));
    window.draw(text);
}
