#include "stats.hpp"

Stats::Stats()
    : currentGeneration(0)
    , populationCount(0)
    , aliveCount(0)
    , averageHunger(0.0f)
    , survivalRate(0.0f) {}

void Stats::record(float survival, int population, float avgHunger) {
    survivalHistory.push_back(survival);
    populationHistory.push_back((float)population);
    hungerHistory.push_back(avgHunger);
    
    trim(survivalHistory);
    trim(populationHistory);
    trim(hungerHistory);
}

void Stats::reset() {
    *this = Stats();
}

void Stats::trim(std::vector<float>& v) {
    while ((int)v.size() > MAX_SIZE) {
        v.erase(v.begin());
    }
}
