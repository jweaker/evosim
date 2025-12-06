#pragma once
#include "core/config.hpp"
#include "core/stats.hpp"
#include "entities/peep.hpp"
#include "entities/food.hpp"
#include "entities/obstacle.hpp"
#include "entities/survival_zone.hpp"
#include "entities/spawn_zone.hpp"
#include <vector>
#include <random>

class Simulation {
public:
    SimConfig config;
    Stats stats;
    
    std::vector<Peep> peeps;
    std::vector<Food> foods;
    std::vector<Obstacle> obstacles;
    std::vector<SurvivalZone> survivalZones;
    std::vector<SpawnZone> spawnZones;
    
    float generationTimer;
    int currentGeneration;
    bool paused;
    float speedMultiplier;
    int selectedPeep;

    Simulation();
    
    void initialize();
    void update(float dt);
    void reset();
    void applyPreset(int index);

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> randomDist;
    
    Peep createRandomPeep();
    Peep createChild(const Peep& parent);
    Peep createChild(const Peep& p1, const Peep& p2);
    
    void updatePeep(Peep& p, float dt);
    void calculateInputs(const Peep& p, float* inputs);
    void movePeep(Peep& p, float mx, float my, float dt);
    void resolveCollisions();
    void checkFood(Peep& p);
    void updateFood(float dt);
    
    void endGeneration();
    std::vector<Peep*> getSurvivors();
    
    Food* findNearestFood(sf::Vector2f pos);
    float distanceToWall(sf::Vector2f pos);
    bool insideObstacle(sf::Vector2f pos);
    bool inSurvivalZone(sf::Vector2f pos);
    void findNearestZone(sf::Vector2f pos, float& dx, float& dy, float& dist, bool& inside);
    sf::Vector2f randomValidPosition();
    
    void spawnFood();
    void generateMazeObstacles();
    void generatePresetZone();
    void generatePresetSpawnZone();
    void updateStats();
};
