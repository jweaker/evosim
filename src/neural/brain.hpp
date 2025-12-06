#pragma once
#include "gene.hpp"
#include <vector>
#include <cstdint>
#include <random>

class Brain {
    std::vector<Gene> genes;
    std::vector<float> hidden;
    std::vector<float> output;
    int hiddenCount;

public:
    Brain();
    
    void init(int numHidden, int numGenes);
    void setGenome(const std::vector<uint32_t>& genome);
    void process(const float* inputs, int inputCount);
    
    float getOutput(int index) const;
    const std::vector<Gene>& getGenes() const { return genes; }
    
    static std::vector<uint32_t> randomGenome(int numGenes, std::mt19937& rng);
    static std::vector<uint32_t> mutate(std::vector<uint32_t> genome, 
                                         float rate, float strength, 
                                         std::mt19937& rng,
                                         std::vector<uint32_t>* mutatedBits = nullptr);
    static std::vector<uint32_t> crossover(const std::vector<uint32_t>& a,
                                            const std::vector<uint32_t>& b,
                                            std::mt19937& rng);
};
