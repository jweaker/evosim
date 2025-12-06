#pragma once
#include <SFML/System/Vector2.hpp>

class Food {
    sf::Vector2f pos;
    bool isEaten;
    float timer;

public:
    Food();
    Food(float x, float y);
    
    sf::Vector2f position() const { return pos; }
    bool eaten() const { return isEaten; }
    float respawnTimer() const { return timer; }
    
    void eat(float respawnTime);
    void update(float dt);
    void respawn(sf::Vector2f newPos);
};
