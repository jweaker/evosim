// simulation.cpp - Core Simulation class methods
// Constructor, initialize, update, reset, applyPreset, updateStats

#include "../simulation.hpp"
#include "../core/constants.hpp"
#include <algorithm>

Simulation::Simulation()
    : generationTimer(0.0f)
    , currentGeneration(0)
    , paused(false)
    , speedMultiplier(1.0f)
    , selectedPeep(-1)
    , randomDist(-1.0f, 1.0f) {
    std::random_device rd;
    rng = std::mt19937(rd());
}

void Simulation::initialize() {
    peeps.clear();
    foods.clear();
    
    currentGeneration = 0;
    generationTimer = 0.0f;
    selectedPeep = -1;
    stats.reset();
    
    int pop = config.getInitialPopulation();
    peeps.reserve(pop);
    for (int i = 0; i < pop; i++) {
        peeps.push_back(createRandomPeep());
    }
    
    if (config.hungerEnabled) {
        spawnFood();
    }
    
    updateStats();
}

void Simulation::update(float deltaTime) {
    if (paused) return;
    
    float simTime = deltaTime * speedMultiplier;
    
    while (simTime > 0.0f) {
        float step = std::min(simTime, Constants::MAX_PHYSICS_STEP);
        simTime -= step;
        
        generationTimer += step;
        
        if (config.hungerEnabled) {
            updateFood(step);
        }
        
        int alive = 0;
        for (auto& p : peeps) {
            if (!p.alive) continue;
            updatePeep(p, step);
            if (p.alive) alive++;
        }
        
        resolveCollisions();
        
        bool shouldEnd = (generationTimer >= config.generationTime);
        
        if (config.hungerEnabled) {
            int minPop = config.getMinPopulation();
            if (alive <= minPop && alive > 0) {
                shouldEnd = true;
            } else if (alive == 0) {
                initialize();
                return;
            }
        }
        
        if (shouldEnd) {
            endGeneration();
            break;
        }
    }
    
    updateStats();
}

void Simulation::reset() {
    initialize();
}

void Simulation::applyPreset(int index) {
    obstacles.clear();
    survivalZones.clear();
    spawnZones.clear();
    
    switch (index) {
        case 0: config.presetMigration(); break;
        case 1: config.presetForaging(); break;
        case 2: config.presetMazeRunner(); generateMazeObstacles(); break;
        case 3: config.presetHungerGames(); generateMazeObstacles(); break;
        case 4: config.presetBigBrains(); generateMazeObstacles(); break;
        case 5: config.presetSpeedDemons(); break;
        default: config.resetToDefaults(); break;
    }
    
    if (config.survivalZoneEnabled) {
        generatePresetZone();
    }
    if (config.spawnOnLeft) {
        generatePresetSpawnZone();
    }
    
    initialize();
}

void Simulation::updateStats() {
    stats.currentGeneration = currentGeneration;
    stats.populationCount = (int)peeps.size();
    
    int alive = 0;
    float hunger = 0.0f;
    for (auto& p : peeps) {
        if (p.alive) {
            alive++;
            hunger += p.hunger;
        }
    }
    
    stats.aliveCount = alive;
    stats.averageHunger = alive > 0 ? hunger / (float)alive : 0.0f;
    stats.survivalRate = peeps.empty() ? 0.0f : (float)alive / (float)peeps.size();
}
