// generation.cpp - Reproduction and survival logic
// createRandomPeep, createChild, endGeneration, getSurvivors

#include "../simulation.hpp"
#include "../neural/brain.hpp"

Peep Simulation::createRandomPeep() {
    Peep p;
    p.position = randomValidPosition();
    p.hunger = config.startingHunger;
    p.generation = currentGeneration;
    
    int genes = config.getGenesPerPeep();
    int hidden = config.getHiddenNeurons();
    
    p.genome = Brain::randomGenome(genes, rng);
    p.init(hidden, genes);
    p.updateColor();
    
    return p;
}

Peep Simulation::createChild(const Peep& parent) {
    Peep child;
    child.position = randomValidPosition();
    child.hunger = config.startingHunger;
    child.generation = currentGeneration;
    
    child.genome = Brain::mutate(parent.genome, config.mutationRate, 
                                  config.mutationStrength, rng, &child.mutatedBits);
    child.init(config.getHiddenNeurons(), config.getGenesPerPeep());
    child.updateColor();
    
    return child;
}

Peep Simulation::createChild(const Peep& p1, const Peep& p2) {
    Peep child;
    child.position = randomValidPosition();
    child.hunger = config.startingHunger;
    child.generation = currentGeneration;
    
    auto mixed = Brain::crossover(p1.genome, p2.genome, rng);
    child.genome = Brain::mutate(mixed, config.mutationRate, 
                                  config.mutationStrength, rng, &child.mutatedBits);
    child.init(config.getHiddenNeurons(), config.getGenesPerPeep());
    child.updateColor();
    
    return child;
}

void Simulation::endGeneration() {
    auto survivors = getSurvivors();
    
    float rate = 0.0f;
    if (!peeps.empty()) {
        if (config.hungerEnabled) {
            int alive = 0;
            for (auto& p : peeps) if (p.alive) alive++;
            rate = (float)alive / (float)peeps.size();
        } else {
            rate = (float)survivors.size() / (float)peeps.size();
        }
    }
    
    float avgHunger = 0.0f;
    for (auto* s : survivors) avgHunger += s->hunger;
    if (!survivors.empty()) avgHunger /= (float)survivors.size();
    
    stats.record(rate, (int)survivors.size(), avgHunger);
    
    if (survivors.empty()) {
        initialize();
        return;
    }
    
    std::vector<Peep> nextGen;
    int target = config.getInitialPopulation();
    nextGen.reserve(target);
    
    std::uniform_int_distribution<int> parentDist(0, (int)survivors.size() - 1);
    std::bernoulli_distribution crossDist(config.crossoverRate);
    
    for (int i = 0; i < target; i++) {
        bool useTwoParents = config.useCrossover && survivors.size() > 1 && crossDist(rng);
        
        if (useTwoParents) {
            int idx1 = parentDist(rng);
            int idx2 = parentDist(rng);
            for (int a = 0; a < 10 && idx2 == idx1; a++) {
                idx2 = parentDist(rng);
            }
            nextGen.push_back(createChild(*survivors[idx1], *survivors[idx2]));
        } else {
            nextGen.push_back(createChild(*survivors[parentDist(rng)]));
        }
    }
    
    peeps = std::move(nextGen);
    
    if (config.hungerEnabled) {
        spawnFood();
    }
    
    currentGeneration++;
    generationTimer = 0.0f;
    selectedPeep = -1;
    updateStats();
}

std::vector<Peep*> Simulation::getSurvivors() {
    std::vector<Peep*> survivors;
    bool useDrawn = config.survivalZoneEnabled && !survivalZones.empty();
    
    for (auto& p : peeps) {
        if (!p.alive) continue;
        
        bool survives = true;
        
        if (useDrawn) {
            survives = inSurvivalZone(p.position);
        } else if (config.survivalZoneEnabled) {
            float half = config.worldSize / 2.0f;
            float quarter = config.worldSize / 4.0f;
            int mode = config.getSurvivalMode();
            
            switch (mode) {
                case 0: survives = (p.position.x >= half); break;
                case 1: survives = (p.position.x < half); break;
                case 2: survives = (p.position.x >= half && p.position.y < half); break;
                case 3: survives = (p.position.x < half && p.position.y < half); break;
                case 4: survives = (p.position.x >= half && p.position.y >= half); break;
                case 5: survives = (p.position.x < half && p.position.y >= half); break;
                case 6: 
                    survives = (p.position.x < quarter && p.position.y < quarter) ||
                              (p.position.x >= config.worldSize - quarter && p.position.y < quarter) ||
                              (p.position.x < quarter && p.position.y >= config.worldSize - quarter) ||
                              (p.position.x >= config.worldSize - quarter && p.position.y >= config.worldSize - quarter);
                    break;
            }
        }
        
        if (survives) survivors.push_back(&p);
    }
    
    return survivors;
}
