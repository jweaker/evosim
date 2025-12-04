// config.hpp - All simulation parameters in one place
// These can be adjusted at runtime through the UI

#pragma once

// ============================================================================
// SIMULATION CONFIGURATION
// All the knobs and dials for the simulation. Change these to experiment!
// ============================================================================

struct SimConfig {
    // --- World Settings ---
    float worldSize = 600.0f;          // Size of the simulation world (square)
    
    // --- Population Settings ---
    float initialPopulation = 100.0f;   // Starting number of peeps
    float maxPopulation = 500.0f;       // Maximum peeps allowed
    float minPopulation = 10.0f;        // Minimum to prevent extinction
    
    // --- Time Settings ---
    float generationTime = 12.0f;       // Seconds per generation
    float simulationSpeed = 1.0f;       // Speed multiplier (1-10)
    
    // --- Neural Network Settings ---
    float genesPerPeep = 12.0f;         // Number of genes (connections) per brain
    float hiddenNeurons = 4.0f;         // Number of internal processing neurons
    
    // --- Mutation Settings ---
    float mutationRate = 0.08f;         // Chance of mutation per gene (0.0 - 1.0)
    float mutationStrength = 0.4f;      // How much a mutation can change a weight
    bool useCrossover = true;           // Mix genes from two parents?
    float crossoverRate = 0.7f;         // Chance of crossover vs cloning
    
    // --- Hunger/Food Settings ---
    bool hungerEnabled = false;         // Use hunger system? (off by default for survival zone demos)
    float startingHunger = 100.0f;      // Hunger peeps start with (100 = full, 0 = starving)
    float maxHunger = 200.0f;           // Maximum hunger a peep can have
    float hungerDecayRate = 2.0f;       // Hunger lost per second
    float movementHungerCost = 0.5f;    // Extra hunger cost for moving
    float reproductionHungerCost = 50.0f; // Hunger needed to reproduce
    
    // --- Food Settings ---
    float foodCount = 80.0f;            // Number of food items in world
    float foodHunger = 30.0f;           // Hunger restored from eating food
    float foodRespawnTime = 2.0f;       // Seconds until eaten food respawns
    float foodSize = 5.0f;              // Visual size of food
    
    // --- Peep Settings ---
    float peepSize = 4.0f;              // Visual size of peeps
    float peepSpeed = 120.0f;           // Base movement speed
    float peepSenseRange = 150.0f;      // How far peeps can "see"
    
    // --- Obstacle Settings ---
    bool obstaclesEnabled = true;       // Use obstacles?
    float obstacleCount = 0.0f;         // Number of auto-generated obstacles (0 = user-drawn only)
    float minObstacleSize = 30.0f;      // Minimum obstacle size
    float maxObstacleSize = 80.0f;      // Maximum obstacle size
    
    // --- Survival Zone Settings ---
    bool survivalZoneEnabled = true;    // Use survival zones? (on by default)
    float survivalMode = 0.0f;          // Which zone pattern to use
    
    // --- Species/Competition Settings ---
    bool speciesEnabled = false;        // Track species by genetic similarity?
    float speciesThreshold = 0.5f;      // How similar genes must be to be same species
    
    // --- Display Settings ---
    bool showSenseRange = false;        // Draw peep vision circles?
    bool showHungerColors = false;      // Color peeps by hunger instead of genes?
    int peepColorMode = 0;              // 0 = by genes, 1 = by hunger (synced with showHungerColors)
    bool showBrainActivity = false;     // Color by neural activity?
    
    // Helper methods to get int values
    int getInitialPopulation() const { return static_cast<int>(initialPopulation); }
    int getMaxPopulation() const { return static_cast<int>(maxPopulation); }
    int getMinPopulation() const { return static_cast<int>(minPopulation); }
    int getGenesPerPeep() const { return static_cast<int>(genesPerPeep); }
    int getHiddenNeurons() const { return static_cast<int>(hiddenNeurons); }
    int getFoodCount() const { return static_cast<int>(foodCount); }
    int getObstacleCount() const { return static_cast<int>(obstacleCount); }
    int getSurvivalMode() const { return static_cast<int>(survivalMode); }
    
    // Reset to defaults
    void resetToDefaults() {
        *this = SimConfig();
    }
};

// ============================================================================
// WINDOW AND UI CONFIGURATION  
// ============================================================================

struct WindowConfig {
    int width = 1400;                   // Window width
    int height = 900;                   // Window height
    int frameLimit = 60;                // Max FPS
    float uiPanelWidth = 350.0f;        // Width of the control panel
    float padding = 10.0f;              // UI padding
};

// ============================================================================
// NEURAL NETWORK CONFIGURATION
// Defines what inputs and outputs the peep brains have
// ============================================================================

// Total neurons in the network
constexpr int INPUT_NEURON_COUNT = 15;   // Sensory inputs
constexpr int OUTPUT_NEURON_COUNT = 2;   // Action outputs
constexpr int MAX_HIDDEN_NEURONS = 32;   // Internal processing

// Input neuron indices (what peeps can sense)
namespace InputNeuron {
    constexpr int PosX = 0;              // Current X position (0-1)
    constexpr int PosY = 1;              // Current Y position (0-1)
    constexpr int NearestFoodDX = 2;     // Direction to nearest food X (-1 to 1)
    constexpr int NearestFoodDY = 3;     // Direction to nearest food Y (-1 to 1)
    constexpr int NearestFoodDist = 4;   // Distance to nearest food (0-1)
    constexpr int NearestWallDist = 5;   // Distance to nearest wall (0-1)
    constexpr int Hunger = 6;            // Current hunger level (0-1)
    constexpr int Age = 7;               // Current age (0-1)
    constexpr int TimeRemaining = 8;     // Time left in generation (1=start, 0=end)
    constexpr int ZoneDX = 9;            // Direction to nearest survival zone X (-1 to 1)
    constexpr int ZoneDY = 10;           // Direction to nearest survival zone Y (-1 to 1)
    constexpr int ZoneDist = 11;         // Distance to nearest survival zone (0-1, 0=inside)
    constexpr int InZone = 12;           // Currently inside a survival zone (0 or 1)
    constexpr int Random = 13;           // Random value for unpredictability
    constexpr int Bias = 14;             // Always 1.0 (constant offset)
}

// Output neuron indices (what peeps can do)
namespace OutputNeuron {
    constexpr int MoveX = 0;             // Movement in X direction
    constexpr int MoveY = 1;             // Movement in Y direction
}
