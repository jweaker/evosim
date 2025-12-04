// types.hpp - Core data structures for the simulation
// Keeping these simple and easy to understand

#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cstdint>
#include <string>

// ============================================================================
// GENE - A single connection in the neural network (32-bit encoded)
// ============================================================================
// 
// Bit layout (optimized for bit-flip mutations):
//   Bits 31-27: source (5 bits) - 0-14 = input neurons, 15-22 = hidden neurons
//   Bits 26-23: dest (4 bits) - 0-7 = hidden neurons, 8-15 = output neurons
//   Bits 22-0:  weight (23 bits) - maps to -4.0 to +4.0 range
//
// This layout means:
//   - Single bit flip in source changes which neuron we read from
//   - Single bit flip in dest changes which neuron we write to  
//   - Bit flips in weight make gradual changes (lower bits = smaller changes)

struct Gene {
    // Decoded values (for runtime use)
    int sourceType;     // 0 = input neuron, 1 = hidden neuron
    int sourceIndex;    // Which input (0-9) or hidden (0-7) neuron
    int destType;       // 0 = hidden neuron, 1 = output neuron
    int destIndex;      // Which hidden (0-7) or output (0-1) neuron
    float weight;       // Connection strength (-4.0 to 4.0)
    
    // Encode gene to 32 bits
    uint32_t encode() const;
    
    // Decode gene from 32 bits
    static Gene decode(uint32_t encoded);
    
    // Get binary string representation (for display)
    static std::string toBinaryString(uint32_t encoded);
    
    // Get a colored/formatted representation showing the gene structure
    static std::string toDisplayString(uint32_t encoded);
};

// ============================================================================
// PEEP - An individual creature in the simulation
// ============================================================================

struct Peep {
    // Position and movement
    sf::Vector2f position;
    sf::Vector2f velocity;
    
    // Brain - stored as encoded genes for easy mutation
    std::vector<uint32_t> genome;
    
    // Track which bits were mutated (for visualization)
    // Each entry is a bitmask showing which bits were flipped in corresponding gene
    std::vector<uint32_t> mutatedBits;
    
    // Neural network state (computed each frame)
    std::vector<float> hiddenNeurons;
    std::vector<float> outputNeurons;
    
    // Life stats
    float hunger;
    float age;
    bool alive;
    int generation;     // Which generation this peep was born in
    int speciesId;      // For tracking genetic groups
    
    // Color (computed from genome)
    uint8_t colorR, colorG, colorB;
    
    // Constructor
    Peep();
};

// ============================================================================
// FOOD - Energy source for peeps
// ============================================================================

struct Food {
    sf::Vector2f position;
    bool eaten;
    float respawnTimer;
    
    Food();
    Food(float x, float y);
};

// ============================================================================
// OBSTACLE - Walls/barriers peeps must navigate around
// ============================================================================

struct Obstacle {
    sf::Vector2f position;  // Top-left corner
    sf::Vector2f size;      // Width and height
    
    Obstacle();
    Obstacle(float x, float y, float w, float h);
    
    // Check if a point is inside this obstacle
    bool contains(sf::Vector2f point) const;
    
    // Get the closest point on obstacle edge to a given point
    sf::Vector2f closestPoint(sf::Vector2f point) const;
};

// ============================================================================
// SURVIVAL ZONE - Areas where peeps must be at end of generation to survive
// ============================================================================

struct SurvivalZone {
    sf::Vector2f position;  // Top-left corner
    sf::Vector2f size;      // Width and height
    
    SurvivalZone();
    SurvivalZone(float x, float y, float w, float h);
    
    // Check if a point is inside this zone
    bool contains(sf::Vector2f point) const;
};

// ============================================================================
// STATISTICS - Track simulation progress
// ============================================================================

struct SimStats {
    int currentGeneration;
    int populationCount;
    int aliveCount;
    float averageHunger;
    float survivalRate;
    float geneticDiversity;
    int speciesCount;
    
    // History for graphs (last N generations)
    std::vector<float> survivalHistory;
    std::vector<float> populationHistory;
    std::vector<float> energyHistory;
    
    SimStats();
    void recordGeneration(float survival, int population, float avgHunger);
};
