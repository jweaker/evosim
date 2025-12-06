#include "food.hpp"

Food::Food() : pos(0.0f, 0.0f), isEaten(false), timer(0.0f) {}

Food::Food(float x, float y) : pos(x, y), isEaten(false), timer(0.0f) {}

void Food::eat(float respawnTime) {
    isEaten = true;
    timer = respawnTime;
}

void Food::update(float dt) {
    if (isEaten && timer > 0) {
        timer -= dt;
    }
}

void Food::respawn(sf::Vector2f newPos) {
    pos = newPos;
    isEaten = false;
    timer = 0.0f;
}
