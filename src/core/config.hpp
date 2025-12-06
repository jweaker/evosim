#pragma once

struct SimConfig {
    float worldSize = 600.0f;
    
    // Population
    float initialPopulation = 100.0f;
    float maxPopulation = 500.0f;
    float minPopulation = 10.0f;
    
    // Time
    float generationTime = 12.0f;
    
    // Neural network
    float genesPerPeep = 12.0f;
    float hiddenNeurons = 4.0f;
    
    // Mutation
    float mutationRate = 0.08f;
    float mutationStrength = 0.4f;
    bool useCrossover = true;
    float crossoverRate = 0.7f;
    
    // Hunger
    bool hungerEnabled = false;
    float startingHunger = 100.0f;
    float maxHunger = 200.0f;
    float hungerDecayRate = 2.0f;
    float movementHungerCost = 0.01f;
    
    // Food
    float foodCount = 80.0f;
    float foodHunger = 30.0f;
    float foodRespawnTime = 2.0f;
    float foodSize = 5.0f;
    
    // Peep
    float peepSize = 4.0f;
    float peepSpeed = 120.0f;
    float peepSenseRange = 150.0f;
    
    // Obstacles
    bool obstaclesEnabled = true;
    float obstacleCount = 0.0f;
    float minObstacleSize = 30.0f;
    float maxObstacleSize = 80.0f;
    
    // Survival zones
    bool survivalZoneEnabled = true;
    float survivalMode = 0.0f;
    
    // Spawn
    bool spawnZonesEnabled = false;
    bool spawnOnLeft = false;
    
    // Display
    bool showSenseRange = false;
    bool showHungerColors = false;
    
    int getInitialPopulation() const { return static_cast<int>(initialPopulation); }
    int getMaxPopulation() const { return static_cast<int>(maxPopulation); }
    int getMinPopulation() const { return static_cast<int>(minPopulation); }
    int getGenesPerPeep() const { return static_cast<int>(genesPerPeep); }
    int getHiddenNeurons() const { return static_cast<int>(hiddenNeurons); }
    int getFoodCount() const { return static_cast<int>(foodCount); }
    int getObstacleCount() const { return static_cast<int>(obstacleCount); }
    int getSurvivalMode() const { return static_cast<int>(survivalMode); }
    
    void resetToDefaults() { *this = SimConfig(); }
    
    void presetMigration();
    void presetForaging();
    void presetMazeRunner();
    void presetHungerGames();
    void presetBigBrains();
    void presetSpeedDemons();
};
