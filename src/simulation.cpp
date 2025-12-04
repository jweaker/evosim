// simulation.cpp - Implementation of the evolution simulation

#include "simulation.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

Simulation::Simulation() 
    : generationTimer(0.0f)
    , currentGeneration(0)
    , paused(false)
    , speedMultiplier(1.0f)
    , selectedPeep(-1)
    , rng(std::random_device{}()) {
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void Simulation::initialize() {
    peeps.clear();
    foods.clear();
    // Note: Don't clear obstacles - user drew them
    // Note: Don't clear survivalZones - user drew them
    
    currentGeneration = 0;
    generationTimer = 0.0f;
    selectedPeep = -1;
    
    // Clear history
    stats = SimStats();
    
    // Don't auto-generate obstacles - only use user-drawn ones
    
    // Create initial population
    int maxPop = static_cast<int>(config.maxPopulation);
    int initPop = static_cast<int>(config.initialPopulation);
    peeps.reserve(static_cast<size_t>(maxPop));
    for (int i = 0; i < initPop; i++) {
        peeps.push_back(createRandomPeep());
    }
    
    // Spawn food
    if (config.hungerEnabled) {
        spawnFood();
    }
    
    updateStats();
}

Peep Simulation::createRandomPeep() {
    Peep peep;
    
    // Random position (not inside obstacles)
    peep.position = getRandomValidPosition();
    peep.velocity = sf::Vector2f(0.0f, 0.0f);
    
    // Initialize hunger (100 = full, 0 = starving)
    peep.hunger = config.startingHunger;
    peep.age = 0.0f;
    peep.alive = true;
    peep.generation = currentGeneration;
    
    // Create random genome using the new bit layout
    int geneCount = static_cast<int>(config.genesPerPeep);
    int hiddenCount = static_cast<int>(config.hiddenNeurons);
    
    std::uniform_int_distribution<uint32_t> fullGeneDist(0, 0xFFFFFFFF);
    
    peep.genome.resize(static_cast<size_t>(geneCount));
    for (int i = 0; i < geneCount; i++) {
        // Generate completely random 32-bit gene
        peep.genome[i] = fullGeneDist(rng);
    }
    
    // Initialize neuron arrays
    peep.hiddenNeurons.resize(static_cast<size_t>(hiddenCount), 0.0f);
    peep.outputNeurons.resize(OUTPUT_NEURON_COUNT, 0.0f);
    
    // Set color based on genome
    updatePeepColor(peep);
    
    return peep;
}

Peep Simulation::createChild(const Peep& parent1) {
    Peep child;
    
    // Random position
    child.position = getRandomValidPosition();
    child.velocity = sf::Vector2f(0.0f, 0.0f);
    
    // Copy parent's genome
    child.genome = parent1.genome;
    
    // Apply bit-flip mutations and track which bits were flipped
    child.mutatedBits = mutateGenome(child.genome);
    
    // Initialize neurons
    int hiddenCount = static_cast<int>(config.hiddenNeurons);
    child.hiddenNeurons.resize(static_cast<size_t>(hiddenCount), 0.0f);
    child.outputNeurons.resize(OUTPUT_NEURON_COUNT, 0.0f);
    
    // Update color
    updatePeepColor(child);
    
    // Reset stats
    child.hunger = config.startingHunger;
    child.age = 0.0f;
    child.alive = true;
    child.generation = currentGeneration;
    
    return child;
}

Peep Simulation::createChild(const Peep& parent1, const Peep& parent2) {
    Peep child;
    
    // Random position
    child.position = getRandomValidPosition();
    child.velocity = sf::Vector2f(0.0f, 0.0f);
    
    // Crossover genomes
    child.genome = crossoverGenomes(parent1.genome, parent2.genome);
    
    // Apply bit-flip mutations and track which bits were flipped
    child.mutatedBits = mutateGenome(child.genome);
    
    // Initialize neurons
    int hiddenCount = static_cast<int>(config.hiddenNeurons);
    child.hiddenNeurons.resize(static_cast<size_t>(hiddenCount), 0.0f);
    child.outputNeurons.resize(OUTPUT_NEURON_COUNT, 0.0f);
    
    // Update color
    updatePeepColor(child);
    
    // Reset stats
    child.hunger = config.startingHunger;
    child.age = 0.0f;
    child.alive = true;
    child.generation = currentGeneration;
    
    return child;
}

void Simulation::spawnFood() {
    foods.clear();
    int foodCnt = static_cast<int>(config.foodCount);
    foods.reserve(static_cast<size_t>(foodCnt));
    
    for (int i = 0; i < foodCnt; i++) {
        sf::Vector2f pos = getRandomValidPosition();
        foods.push_back(Food(pos.x, pos.y));
    }
}

void Simulation::createObstacles() {
    obstacles.clear();
    int obsCnt = static_cast<int>(config.obstacleCount);
    obstacles.reserve(static_cast<size_t>(obsCnt));
    
    std::uniform_real_distribution<float> sizeDist(config.minObstacleSize, config.maxObstacleSize);
    
    for (int i = 0; i < obsCnt; i++) {
        float w = sizeDist(rng);
        float h = sizeDist(rng);
        
        float margin = 50.0f;
        std::uniform_real_distribution<float> xDist(margin, config.worldSize - w - margin);
        std::uniform_real_distribution<float> yDist(margin, config.worldSize - h - margin);
        
        obstacles.push_back(Obstacle(xDist(rng), yDist(rng), w, h));
    }
}

// ============================================================================
// SIMULATION UPDATE
// ============================================================================

void Simulation::update(float deltaTime) {
    if (paused) return;
    
    // Total simulated time this frame
    float totalSimTime = deltaTime * speedMultiplier;
    
    // Maximum time step for stable physics (larger = faster but less accurate)
    const float maxStepDt = 0.1f;
    
    // Run multiple simulation steps if needed for high speeds
    while (totalSimTime > 0.0f) {
        float stepDt = std::min(totalSimTime, maxStepDt);
        totalSimTime -= stepDt;
        
        // Update generation timer
        generationTimer += stepDt;
        
        // Update food respawning
        if (config.hungerEnabled) {
            updateFood(stepDt);
        }
        
        // Update each peep
        int aliveCount = 0;
        for (Peep& peep : peeps) {
            if (!peep.alive) continue;
            
            updatePeep(peep, stepDt);
            
            if (peep.alive) {
                aliveCount++;
            }
        }
        
        // Check for generation end
        bool shouldEndGeneration = false;
        
        // Timer always ends the generation when time runs out
        if (generationTimer >= config.generationTime) {
            shouldEndGeneration = true;
        }
        
        // Hunger mode: also end when population drops too low
        if (config.hungerEnabled) {
            int minPop = static_cast<int>(config.minPopulation);
            if (aliveCount <= minPop && aliveCount > 0) {
                shouldEndGeneration = true;
            } else if (aliveCount == 0) {
                // Everyone died - restart
                initialize();
                return;
            }
        }
        
        if (shouldEndGeneration) {
            endGeneration();
            // Don't continue simulating old generation's remaining time
            break;
        }
    }
    
    updateStats();
}

void Simulation::updatePeep(Peep& peep, float dt) {
    // Update age
    peep.age += dt;
    
    // Hunger decay (hunger decreases over time, peep dies when it hits 0)
    if (config.hungerEnabled) {
        peep.hunger -= config.hungerDecayRate * dt;
        
        if (peep.hunger <= 0) {
            peep.alive = false;
            return;
        }
    }
    
    // Calculate inputs
    std::vector<float> inputs = calculateInputs(peep);
    
    // Run brain
    std::vector<float> outputs = runBrain(peep, inputs);
    
    // Apply movement
    float moveX = outputs[OutputNeuron::MoveX];
    float moveY = outputs[OutputNeuron::MoveY];
    movePeep(peep, moveX, moveY, dt);
    
    // Check for food
    if (config.hungerEnabled) {
        checkFoodCollision(peep);
    }
}

std::vector<float> Simulation::calculateInputs(const Peep& peep) {
    std::vector<float> inputs(INPUT_NEURON_COUNT, 0.0f);
    
    // Position (normalized 0-1)
    inputs[InputNeuron::PosX] = peep.position.x / config.worldSize;
    inputs[InputNeuron::PosY] = peep.position.y / config.worldSize;
    
    // Nearest food
    Food* nearestFood = findNearestFood(peep.position);
    if (nearestFood && !nearestFood->eaten) {
        sf::Vector2f toFood = nearestFood->position - peep.position;
        float dist = std::sqrt(toFood.x * toFood.x + toFood.y * toFood.y);
        
        if (dist > 0.001f) {
            inputs[InputNeuron::NearestFoodDX] = toFood.x / dist;
            inputs[InputNeuron::NearestFoodDY] = toFood.y / dist;
        }
        
        inputs[InputNeuron::NearestFoodDist] = std::min(1.0f, dist / config.peepSenseRange);
    } else {
        inputs[InputNeuron::NearestFoodDist] = 1.0f;
    }
    
    // Distance to nearest wall
    inputs[InputNeuron::NearestWallDist] = distanceToNearestWall(peep.position) / config.peepSenseRange;
    
    // Hunger (normalized 0-1, 1 = full, 0 = starving)
    if (config.hungerEnabled) {
        inputs[InputNeuron::Hunger] = peep.hunger / config.maxHunger;
    } else {
        inputs[InputNeuron::Hunger] = 1.0f;
    }
    
    // Age (normalized)
    inputs[InputNeuron::Age] = std::min(1.0f, peep.age / config.generationTime);
    
    // Time remaining in generation (1.0 = just started, 0.0 = about to end)
    float timeRemaining = 1.0f - (generationTimer / config.generationTime);
    inputs[InputNeuron::TimeRemaining] = std::max(0.0f, std::min(1.0f, timeRemaining));
    
    // Survival zone inputs
    float zoneDirX, zoneDirY, zoneDist;
    bool inZone;
    findNearestSurvivalZone(peep.position, zoneDirX, zoneDirY, zoneDist, inZone);
    inputs[InputNeuron::ZoneDX] = zoneDirX;
    inputs[InputNeuron::ZoneDY] = zoneDirY;
    inputs[InputNeuron::ZoneDist] = zoneDist;
    inputs[InputNeuron::InZone] = inZone ? 1.0f : 0.0f;
    
    // Random input
    std::uniform_real_distribution<float> randDist(-1.0f, 1.0f);
    inputs[InputNeuron::Random] = randDist(rng);
    
    // Bias
    inputs[InputNeuron::Bias] = 1.0f;
    
    return inputs;
}

std::vector<float> Simulation::runBrain(Peep& peep, const std::vector<float>& inputs) {
    // Reset neurons
    std::fill(peep.hiddenNeurons.begin(), peep.hiddenNeurons.end(), 0.0f);
    std::fill(peep.outputNeurons.begin(), peep.outputNeurons.end(), 0.0f);
    
    // Temporary accumulators for the next layer
    std::vector<float> hiddenAccum(peep.hiddenNeurons.size(), 0.0f);
    std::vector<float> outputAccum(OUTPUT_NEURON_COUNT, 0.0f);
    
    // PASS 1: Process all input->hidden and input->output connections
    for (uint32_t encoded : peep.genome) {
        Gene gene = Gene::decode(encoded);
        
        // Only process connections FROM input neurons in this pass
        if (gene.sourceType != 0) continue;
        
        // Get input value
        float sourceValue = 0.0f;
        if (gene.sourceIndex < INPUT_NEURON_COUNT) {
            sourceValue = inputs[gene.sourceIndex];
        }
        
        // Apply weight and add to destination accumulator
        float contribution = sourceValue * gene.weight;
        
        if (gene.destType == 0) {
            // To hidden neuron
            if (gene.destIndex < static_cast<int>(hiddenAccum.size())) {
                hiddenAccum[gene.destIndex] += contribution;
            }
        } else {
            // To output neuron
            if (gene.destIndex < OUTPUT_NEURON_COUNT) {
                outputAccum[gene.destIndex] += contribution;
            }
        }
    }
    
    // Apply activation to hidden neurons
    for (size_t i = 0; i < peep.hiddenNeurons.size(); i++) {
        peep.hiddenNeurons[i] = std::tanh(hiddenAccum[i]);
    }
    
    // PASS 2: Process all hidden->hidden and hidden->output connections
    for (uint32_t encoded : peep.genome) {
        Gene gene = Gene::decode(encoded);
        
        // Only process connections FROM hidden neurons in this pass
        if (gene.sourceType != 1) continue;
        
        // Get hidden neuron value (already activated)
        float sourceValue = 0.0f;
        if (gene.sourceIndex < static_cast<int>(peep.hiddenNeurons.size())) {
            sourceValue = peep.hiddenNeurons[gene.sourceIndex];
        }
        
        // Apply weight and add to destination
        float contribution = sourceValue * gene.weight;
        
        if (gene.destType == 0) {
            // Hidden to hidden - add to hidden neuron directly (will be re-processed)
            // This creates recurrent connections within the hidden layer
            if (gene.destIndex < static_cast<int>(peep.hiddenNeurons.size())) {
                peep.hiddenNeurons[gene.destIndex] = std::tanh(peep.hiddenNeurons[gene.destIndex] + contribution);
            }
        } else {
            // To output neuron
            if (gene.destIndex < OUTPUT_NEURON_COUNT) {
                outputAccum[gene.destIndex] += contribution;
            }
        }
    }
    
    // Apply activation to output neurons
    for (int i = 0; i < OUTPUT_NEURON_COUNT; i++) {
        peep.outputNeurons[i] = std::tanh(outputAccum[i]);
    }
    
    return peep.outputNeurons;
}

void Simulation::movePeep(Peep& peep, float moveX, float moveY, float dt) {
    float speed = config.peepSpeed * dt;
    sf::Vector2f newPos = peep.position;
    newPos.x += moveX * speed;
    newPos.y += moveY * speed;
    
    // Clamp to world bounds
    newPos.x = std::max(0.0f, std::min(newPos.x, config.worldSize));
    newPos.y = std::max(0.0f, std::min(newPos.y, config.worldSize));
    
    // Check obstacle collision
    if (config.obstaclesEnabled && isInsideObstacle(newPos)) {
        sf::Vector2f tryX = sf::Vector2f(newPos.x, peep.position.y);
        sf::Vector2f tryY = sf::Vector2f(peep.position.x, newPos.y);
        
        if (!isInsideObstacle(tryX)) {
            newPos = tryX;
        } else if (!isInsideObstacle(tryY)) {
            newPos = tryY;
        } else {
            newPos = peep.position;
        }
    }
    
    // Hunger cost for movement (moving uses more energy, decreases hunger faster)
    sf::Vector2f movement = newPos - peep.position;
    float moveDist = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    
    if (config.hungerEnabled && moveDist > 0.01f && speed > 0.0001f) {
        peep.hunger -= config.movementHungerCost * moveDist / speed * dt;
    }
    
    peep.position = newPos;
    if (dt > 0.0001f) {
        peep.velocity = movement / dt;
    }
}

void Simulation::checkFoodCollision(Peep& peep) {
    float eatRadius = config.peepSize + config.foodSize;
    
    for (Food& food : foods) {
        if (food.eaten) continue;
        
        sf::Vector2f toFood = food.position - peep.position;
        float dist = std::sqrt(toFood.x * toFood.x + toFood.y * toFood.y);
        
        if (dist < eatRadius) {
            food.eaten = true;
            food.respawnTimer = config.foodRespawnTime;
            peep.hunger = std::min(peep.hunger + config.foodHunger, config.maxHunger);
        }
    }
}

void Simulation::updateFood(float dt) {
    for (Food& food : foods) {
        if (!food.eaten) continue;
        
        food.respawnTimer -= dt;
        if (food.respawnTimer <= 0) {
            sf::Vector2f newPos = getRandomValidPosition();
            food.position = newPos;
            food.eaten = false;
        }
    }
}

// ============================================================================
// REPRODUCTION AND EVOLUTION
// ============================================================================

void Simulation::endGeneration() {
    // Get survivors
    std::vector<Peep*> survivors = getSurvivors();
    
    // Calculate survival rate
    float survivalRate = 0.0f;
    if (!peeps.empty()) {
        if (config.hungerEnabled) {
            int aliveCount = 0;
            for (const Peep& p : peeps) {
                if (p.alive) aliveCount++;
            }
            survivalRate = static_cast<float>(aliveCount) / static_cast<float>(peeps.size());
        } else {
            survivalRate = static_cast<float>(survivors.size()) / static_cast<float>(peeps.size());
        }
    }
    
    // Record stats
    float avgHunger = 0.0f;
    for (const Peep* s : survivors) {
        avgHunger += s->hunger;
    }
    if (!survivors.empty()) {
        avgHunger /= static_cast<float>(survivors.size());
    }
    stats.recordGeneration(survivalRate, static_cast<int>(survivors.size()), avgHunger);
    
    // If no survivors, restart
    if (survivors.empty()) {
        initialize();
        return;
    }
    
    // Create next generation
    std::vector<Peep> nextGen;
    int initPop = static_cast<int>(config.initialPopulation);
    nextGen.reserve(static_cast<size_t>(initPop));
    
    std::uniform_int_distribution<size_t> parentDist(0, survivors.size() - 1);
    std::bernoulli_distribution crossoverDist(config.crossoverRate);
    
    for (int i = 0; i < initPop; i++) {
        Peep child;
        if (config.useCrossover && survivors.size() > 1 && crossoverDist(rng)) {
            size_t idx1 = parentDist(rng);
            size_t idx2 = parentDist(rng);
            int attempts = 0;
            while (idx2 == idx1 && attempts < 10) {
                idx2 = parentDist(rng);
                attempts++;
            }
            child = createChild(*survivors[idx1], *survivors[idx2]);
        } else {
            size_t idx = parentDist(rng);
            child = createChild(*survivors[idx]);
        }
        
        nextGen.push_back(child);
    }
    
    // Replace population
    peeps = std::move(nextGen);
    
    // Respawn food
    if (config.hungerEnabled) {
        spawnFood();
    }
    
    // Advance generation
    currentGeneration++;
    generationTimer = 0.0f;
    selectedPeep = -1;
    
    updateStats();
}

std::vector<Peep*> Simulation::getSurvivors() {
    std::vector<Peep*> survivors;
    
    // First, check if survival zones are enabled and we have zones
    bool useSurvivalZones = config.survivalZoneEnabled && !survivalZones.empty();
    
    for (Peep& p : peeps) {
        if (!p.alive) continue;
        
        bool survives = true;
        
        // If survival zones are enabled, peep must be inside a zone
        if (useSurvivalZones) {
            survives = isInSurvivalZone(p.position);
        }
        // If using preset survival modes (no drawn zones but mode enabled)
        else if (config.survivalZoneEnabled) {
            float half = config.worldSize / 2.0f;
            float quarter = config.worldSize / 4.0f;
            int mode = static_cast<int>(config.survivalMode);
            
            switch (mode) {
                case 0: survives = p.position.x >= half; break;
                case 1: survives = p.position.x < half; break;
                case 2: survives = p.position.x >= half && p.position.y < half; break;
                case 3: survives = p.position.x < half && p.position.y < half; break;
                case 4: survives = p.position.x >= half && p.position.y >= half; break;
                case 5: survives = p.position.x < half && p.position.y >= half; break;
                case 6: survives = (p.position.x < quarter && p.position.y < quarter) ||
                                   (p.position.x >= config.worldSize - quarter && p.position.y < quarter) ||
                                   (p.position.x < quarter && p.position.y >= config.worldSize - quarter) ||
                                   (p.position.x >= config.worldSize - quarter && p.position.y >= config.worldSize - quarter);
                        break;
                default: survives = true; break;
            }
        }
        // Energy mode with no survival zones: survival based on having energy
        // (already filtered by p.alive which requires energy > 0)
        
        if (survives) {
            survivors.push_back(&p);
        }
    }
    
    return survivors;
}

// ============================================================================
// BIT-FLIP MUTATION
// ============================================================================

std::vector<uint32_t> Simulation::mutateGenome(std::vector<uint32_t>& genome) {
    // Mutation rate is per-gene probability
    // For each gene, we might flip one or more bits
    
    std::bernoulli_distribution shouldMutate(config.mutationRate);
    std::uniform_int_distribution<int> bitDist(0, 31);
    
    // How many bits to flip when mutation occurs (based on mutation strength)
    // strength 0.1 = usually 1 bit, strength 1.0 = up to 3-4 bits
    int maxBitFlips = std::max(1, static_cast<int>(config.mutationStrength * 4));
    std::uniform_int_distribution<int> flipCountDist(1, maxBitFlips);
    
    // Track which bits were flipped for each gene
    std::vector<uint32_t> mutationMask(genome.size(), 0);
    
    for (size_t i = 0; i < genome.size(); i++) {
        if (shouldMutate(rng)) {
            // Flip random bits
            int numFlips = flipCountDist(rng);
            for (int f = 0; f < numFlips; f++) {
                int bit = bitDist(rng);
                genome[i] ^= (1u << bit);      // XOR flips the bit
                mutationMask[i] |= (1u << bit); // Record which bit was flipped
            }
        }
    }
    
    return mutationMask;
}

std::vector<uint32_t> Simulation::crossoverGenomes(const std::vector<uint32_t>& g1, 
                                                    const std::vector<uint32_t>& g2) {
    std::vector<uint32_t> child;
    size_t size = std::min(g1.size(), g2.size());
    child.reserve(size);
    
    std::bernoulli_distribution coinFlip(0.5);
    
    for (size_t i = 0; i < size; i++) {
        if (coinFlip(rng)) {
            child.push_back(g1[i]);
        } else {
            child.push_back(g2[i]);
        }
    }
    
    return child;
}

void Simulation::updatePeepColor(Peep& peep) {
    // Color based on behavioral traits extracted from genome
    // This makes peeps with similar DNA have similar colors,
    // and the colors reflect actual behavior tendencies
    
    // Behavioral trait accumulators
    float foodInfluence = 0.0f;    // How much food inputs affect movement
    float zoneInfluence = 0.0f;    // How much zone inputs affect movement
    float posInfluence = 0.0f;     // How much position affects movement
    float randomInfluence = 0.0f;  // How much random/bias affects movement
    float totalWeight = 0.0f;      // Total absolute weight (activity level)
    
    // Analyze each gene to understand behavioral tendencies
    for (uint32_t encoded : peep.genome) {
        Gene gene = Gene::decode(encoded);
        float absWeight = std::abs(gene.weight);
        
        // Only count genes that connect to output neurons (actual behavior)
        // or through hidden neurons (indirect influence)
        bool affectsOutput = (gene.destType == 1); // Direct to output
        bool fromInput = (gene.sourceType == 0);   // From input neuron
        
        if (fromInput) {
            int src = gene.sourceIndex;
            float influence = absWeight;
            
            // Categorize by input type
            if (src == InputNeuron::NearestFoodDX || 
                src == InputNeuron::NearestFoodDY || 
                src == InputNeuron::NearestFoodDist ||
                src == InputNeuron::Hunger) {
                // Food-related inputs
                foodInfluence += influence * (affectsOutput ? 2.0f : 1.0f);
            }
            else if (src == InputNeuron::ZoneDX || 
                     src == InputNeuron::ZoneDY || 
                     src == InputNeuron::ZoneDist ||
                     src == InputNeuron::InZone ||
                     src == InputNeuron::TimeRemaining) {
                // Zone/survival-related inputs
                zoneInfluence += influence * (affectsOutput ? 2.0f : 1.0f);
            }
            else if (src == InputNeuron::PosX || 
                     src == InputNeuron::PosY ||
                     src == InputNeuron::NearestWallDist) {
                // Position/spatial awareness
                posInfluence += influence * (affectsOutput ? 2.0f : 1.0f);
            }
            else if (src == InputNeuron::Random || 
                     src == InputNeuron::Bias) {
                // Random/constant behavior
                randomInfluence += influence * (affectsOutput ? 2.0f : 1.0f);
            }
        }
        
        totalWeight += absWeight;
    }
    
    // Normalize influences
    float totalInfluence = foodInfluence + zoneInfluence + posInfluence + randomInfluence;
    if (totalInfluence < 0.001f) totalInfluence = 1.0f;
    
    float foodRatio = foodInfluence / totalInfluence;
    float zoneRatio = zoneInfluence / totalInfluence;
    float posRatio = posInfluence / totalInfluence;
    float randomRatio = randomInfluence / totalInfluence;
    
    // Map behavioral profile to color
    // Hue: Primary behavior type
    //   Red/Orange (0-60): Food-seeking (survival instinct)
    //   Yellow/Green (60-150): Zone-seeking (goal-oriented)
    //   Cyan/Blue (150-240): Position-aware (spatial)
    //   Purple/Magenta (240-330): Random/exploratory
    //   Blended based on ratios
    
    float hue = 0.0f;
    hue += foodRatio * 30.0f;      // Orange center
    hue += zoneRatio * 105.0f;     // Green center  
    hue += posRatio * 195.0f;      // Cyan center
    hue += randomRatio * 285.0f;   // Purple center
    
    // Add some variation based on specific gene patterns for siblings to differ slightly
    uint32_t geneHash = 0;
    for (size_t i = 0; i < peep.genome.size(); i++) {
        geneHash ^= peep.genome[i] >> 16; // Use weight bits for subtle variation
    }
    float hueVariation = static_cast<float>(geneHash % 40) - 20.0f; // +/- 20 degrees
    hue = std::fmod(hue + hueVariation + 360.0f, 360.0f);
    
    // Saturation: How specialized/focused the behavior is
    // High saturation = one dominant behavior, low = generalist
    float maxRatio = std::max({foodRatio, zoneRatio, posRatio, randomRatio});
    float saturation = 0.4f + maxRatio * 0.5f; // 0.4 to 0.9
    
    // Lightness: Activity level (total connection strength)
    // More active networks are brighter
    float avgWeight = totalWeight / std::max(1.0f, static_cast<float>(peep.genome.size()));
    float lightness = 0.35f + std::min(avgWeight / 3.0f, 0.35f); // 0.35 to 0.7
    
    // HSL to RGB conversion
    float c = (1.0f - std::abs(2.0f * lightness - 1.0f)) * saturation;
    float x = c * (1.0f - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = lightness - c / 2.0f;
    
    float r = 0, g = 0, b = 0;
    if (hue < 60)       { r = c; g = x; }
    else if (hue < 120) { r = x; g = c; }
    else if (hue < 180) { g = c; b = x; }
    else if (hue < 240) { g = x; b = c; }
    else if (hue < 300) { r = x; b = c; }
    else                { r = c; b = x; }
    
    peep.colorR = static_cast<uint8_t>(std::min(255.0f, (r + m) * 255.0f));
    peep.colorG = static_cast<uint8_t>(std::min(255.0f, (g + m) * 255.0f));
    peep.colorB = static_cast<uint8_t>(std::min(255.0f, (b + m) * 255.0f));
}

// ============================================================================
// UTILITIES
// ============================================================================

Food* Simulation::findNearestFood(sf::Vector2f pos) {
    Food* nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();
    
    for (Food& food : foods) {
        if (food.eaten) continue;
        
        sf::Vector2f toFood = food.position - pos;
        float dist = toFood.x * toFood.x + toFood.y * toFood.y;
        
        if (dist < minDist) {
            minDist = dist;
            nearest = &food;
        }
    }
    
    return nearest;
}

float Simulation::distanceToNearestWall(sf::Vector2f pos) {
    float distToEdge = std::min({
        pos.x,
        pos.y,
        config.worldSize - pos.x,
        config.worldSize - pos.y
    });
    
    for (const Obstacle& obs : obstacles) {
        sf::Vector2f closest = obs.closestPoint(pos);
        sf::Vector2f toObs = closest - pos;
        float dist = std::sqrt(toObs.x * toObs.x + toObs.y * toObs.y);
        distToEdge = std::min(distToEdge, dist);
    }
    
    return distToEdge;
}

bool Simulation::isInsideObstacle(sf::Vector2f pos) {
    for (const Obstacle& obs : obstacles) {
        if (obs.contains(pos)) {
            return true;
        }
    }
    return false;
}

bool Simulation::isInSurvivalZone(sf::Vector2f pos) {
    for (const SurvivalZone& zone : survivalZones) {
        if (zone.contains(pos)) {
            return true;
        }
    }
    return false;
}

void Simulation::findNearestSurvivalZone(sf::Vector2f pos, float& dirX, float& dirY, float& dist, bool& inside) {
    dirX = 0.0f;
    dirY = 0.0f;
    dist = 1.0f;  // Max distance (normalized)
    inside = false;
    
    if (survivalZones.empty()) {
        // No zones defined - use preset mode if enabled
        if (config.survivalZoneEnabled) {
            float half = config.worldSize / 2.0f;
            int mode = static_cast<int>(config.survivalMode);
            
            // Default preset: right half of world
            sf::Vector2f target;
            switch (mode) {
                case 0: // Right half
                    target = sf::Vector2f(config.worldSize * 0.75f, config.worldSize * 0.5f);
                    inside = pos.x >= half;
                    break;
                case 1: // Left half
                    target = sf::Vector2f(config.worldSize * 0.25f, config.worldSize * 0.5f);
                    inside = pos.x < half;
                    break;
                default:
                    target = sf::Vector2f(config.worldSize * 0.75f, config.worldSize * 0.5f);
                    inside = pos.x >= half;
                    break;
            }
            
            sf::Vector2f toTarget = target - pos;
            float d = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
            if (d > 0.001f) {
                dirX = toTarget.x / d;
                dirY = toTarget.y / d;
            }
            dist = inside ? 0.0f : std::min(1.0f, d / config.worldSize);
        }
        return;
    }
    
    // Find nearest zone
    float minDist = std::numeric_limits<float>::max();
    sf::Vector2f nearestPoint;
    
    for (const SurvivalZone& zone : survivalZones) {
        // Check if inside this zone
        if (zone.contains(pos)) {
            inside = true;
            dist = 0.0f;
            // Direction to center of zone
            sf::Vector2f center = zone.position + zone.size * 0.5f;
            sf::Vector2f toCenter = center - pos;
            float d = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (d > 0.001f) {
                dirX = toCenter.x / d;
                dirY = toCenter.y / d;
            }
            return;
        }
        
        // Find closest point on zone edge
        float closestX = std::max(zone.position.x, std::min(pos.x, zone.position.x + zone.size.x));
        float closestY = std::max(zone.position.y, std::min(pos.y, zone.position.y + zone.size.y));
        
        sf::Vector2f closest(closestX, closestY);
        sf::Vector2f toZone = closest - pos;
        float d = std::sqrt(toZone.x * toZone.x + toZone.y * toZone.y);
        
        if (d < minDist) {
            minDist = d;
            nearestPoint = closest;
        }
    }
    
    // Calculate direction to nearest zone
    sf::Vector2f toNearest = nearestPoint - pos;
    float d = std::sqrt(toNearest.x * toNearest.x + toNearest.y * toNearest.y);
    if (d > 0.001f) {
        dirX = toNearest.x / d;
        dirY = toNearest.y / d;
    }
    dist = std::min(1.0f, minDist / config.peepSenseRange);
}

sf::Vector2f Simulation::getRandomValidPosition() {
    std::uniform_real_distribution<float> posDist(10.0f, config.worldSize - 10.0f);
    
    sf::Vector2f pos;
    int attempts = 0;
    const int maxAttempts = 100;
    
    do {
        pos = sf::Vector2f(posDist(rng), posDist(rng));
        attempts++;
    } while (isInsideObstacle(pos) && attempts < maxAttempts);
    
    return pos;
}

void Simulation::updateStats() {
    stats.currentGeneration = currentGeneration;
    stats.populationCount = static_cast<int>(peeps.size());
    
    int aliveCount = 0;
    float totalHunger = 0.0f;
    
    for (const Peep& p : peeps) {
        if (p.alive) {
            aliveCount++;
            totalHunger += p.hunger;
        }
    }
    
    stats.aliveCount = aliveCount;
    stats.averageHunger = aliveCount > 0 ? totalHunger / static_cast<float>(aliveCount) : 0.0f;
    stats.survivalRate = peeps.empty() ? 0.0f : static_cast<float>(aliveCount) / static_cast<float>(peeps.size());
}

void Simulation::reset() {
    initialize();
}
