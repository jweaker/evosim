#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Slider {
public:
    sf::FloatRect bounds;
    float minValue;
    float maxValue;
    float* value;
    std::string label;
    bool isInteger;
    bool isLog;
    bool dragging;

    Slider();
    Slider(float x, float y, float w, float h, float minV, float maxV, 
           float* val, const std::string& lbl, bool isInt = false, bool log = false);

    bool handleClick(sf::Vector2f pos);
    void handleDrag(sf::Vector2f pos);
    void draw(sf::RenderWindow& window, const sf::Font& font);
};

class Toggle {
public:
    sf::FloatRect bounds;
    bool* value;
    std::string label;

    Toggle();
    Toggle(float x, float y, float w, float h, bool* val, const std::string& lbl);

    bool handleClick(sf::Vector2f pos);
    void draw(sf::RenderWindow& window, const sf::Font& font);
};

class Button {
public:
    sf::FloatRect bounds;
    std::string label;

    Button();
    Button(float x, float y, float w, float h, const std::string& lbl);

    bool handleClick(sf::Vector2f pos);
    void draw(sf::RenderWindow& window, const sf::Font& font, bool highlighted = false);
};
