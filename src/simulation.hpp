// simulation.hpp - Main simulation logic
// Manages all peeps, food, obstacles, and the simulation loop

#pragma once
#include "types.hpp"
#include "config.hpp"
#include <vector>
#include <random>

// ============================================================================
// SIMULATION CLASS
// The heart of the evolution simulator
// ============================================================================

class Simulation {
public:
    // Configuration (can be modified through UI)
    SimConfig config;
    
    // Simulation state
    std::vector<Peep> peeps;
    std::vector<Food> foods;
    std::vector<Obstacle> obstacles;
    std::vector<SurvivalZone> survivalZones;
    SimStats stats;
    
    // Time tracking
    float generationTimer;
    int currentGeneration;
    bool paused;
    float speedMultiplier;  // Now float for proper slider binding
    
    // Selected peep for inspection (index or -1 if none)
    int selectedPeep;
    
    // Random number generator
    std::mt19937 rng;
    
    // Constructor
    Simulation();
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    // Set up a fresh simulation
    void initialize();
    
    // Create a single peep with random genome
    Peep createRandomPeep();
    
    // Create a child peep from one or two parents
    Peep createChild(const Peep& parent1);
    Peep createChild(const Peep& parent1, const Peep& parent2);
    
    // Spawn food items
    void spawnFood();
    
    // Create obstacles
    void createObstacles();
    
    // ========================================================================
    // SIMULATION UPDATE
    // ========================================================================
    
    // Main update function - call once per frame
    void update(float deltaTime);
    
    // Update a single peep's brain and movement
    void updatePeep(Peep& peep, float dt);
    
    // Calculate neural network inputs for a peep
    std::vector<float> calculateInputs(const Peep& peep);
    
    // Run the neural network and get outputs
    std::vector<float> runBrain(Peep& peep, const std::vector<float>& inputs);
    
    // Handle peep movement and collisions
    void movePeep(Peep& peep, float moveX, float moveY, float dt);
    
    // Check if peep can eat nearby food
    void checkFoodCollision(Peep& peep);
    
    // Update food respawning
    void updateFood(float dt);
    
    // ========================================================================
    // REPRODUCTION AND EVOLUTION
    // ========================================================================
    
    // End of generation - select survivors and create next generation
    void endGeneration();
    
    // Get list of peeps that will reproduce
    std::vector<Peep*> getSurvivors();
    
    // Mutate a genome, returns bitmask of which bits were flipped per gene
    std::vector<uint32_t> mutateGenome(std::vector<uint32_t>& genome);
    
    // Crossover two genomes
    std::vector<uint32_t> crossoverGenomes(const std::vector<uint32_t>& g1, 
                                            const std::vector<uint32_t>& g2);
    
    // Calculate color from genome
    void updatePeepColor(Peep& peep);
    
    // ========================================================================
    // UTILITIES
    // ========================================================================
    
    // Find nearest food to a position
    Food* findNearestFood(sf::Vector2f pos);
    
    // Get distance to nearest wall
    float distanceToNearestWall(sf::Vector2f pos);
    
    // Check if position is inside any obstacle
    bool isInsideObstacle(sf::Vector2f pos);
    
    // Check if position is inside any survival zone
    bool isInSurvivalZone(sf::Vector2f pos);
    
    // Find nearest survival zone and return direction/distance
    // Returns: {dirX, dirY, distance, isInside}
    void findNearestSurvivalZone(sf::Vector2f pos, float& dirX, float& dirY, float& dist, bool& inside);
    
    // Get a random position not inside an obstacle
    sf::Vector2f getRandomValidPosition();
    
    // Update statistics
    void updateStats();
    
    // Reset the simulation
    void reset();
};
