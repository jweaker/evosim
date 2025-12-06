#pragma once
#include "../neural/brain.hpp"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cstdint>

class Peep {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    
    std::vector<uint32_t> genome;
    std::vector<uint32_t> mutatedBits;
    Brain brain;
    
    float hunger;
    float age;
    bool alive;
    int generation;
    
    unsigned char colorR;
    unsigned char colorG;
    unsigned char colorB;

    Peep();
    
    void init(int numHidden, int numGenes);
    void updateColor();
};
