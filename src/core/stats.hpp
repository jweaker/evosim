#pragma once
#include <vector>

class Stats {
public:
    int currentGeneration;
    int populationCount;
    int aliveCount;
    float averageHunger;
    float survivalRate;
    
    std::vector<float> survivalHistory;
    std::vector<float> populationHistory;
    std::vector<float> hungerHistory;
    
    Stats();
    void record(float survival, int population, float avgHunger);
    void reset();

private:
    static const int MAX_SIZE = 100;
    void trim(std::vector<float>& v);
};
