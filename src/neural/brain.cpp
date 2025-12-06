#include "brain.hpp"
#include "../core/constants.hpp"
#include <cmath>

Brain::Brain() : hiddenCount(0) {}

void Brain::init(int numHidden, int numGenes) {
    hiddenCount = numHidden;
    hidden.resize(numHidden, 0.0f);
    output.resize(NeuralConfig::OUTPUT_COUNT, 0.0f);
    genes.reserve(numGenes);
}

void Brain::setGenome(const std::vector<uint32_t>& genome) {
    genes.clear();
    for (auto& g : genome) {
        genes.push_back(Gene::decode(g));
    }
}

void Brain::process(const float* inputs, int inputCount) {
    for (auto& h : hidden) h = 0.0f;
    for (auto& o : output) o = 0.0f;
    
    std::vector<float> hiddenAcc(hidden.size(), 0.0f);
    std::vector<float> outputAcc(output.size(), 0.0f);
    
    // Pass 1: inputs -> hidden/output
    for (auto& g : genes) {
        if (g.sourceType != 0) continue;
        
        float val = (g.sourceIndex < inputCount) ? inputs[g.sourceIndex] : 0.0f;
        float contrib = val * g.weight;
        
        if (g.destType == 0 && g.destIndex < (int)hiddenAcc.size()) {
            hiddenAcc[g.destIndex] += contrib;
        } else if (g.destType == 1 && g.destIndex < (int)outputAcc.size()) {
            outputAcc[g.destIndex] += contrib;
        }
    }
    
    for (size_t i = 0; i < hidden.size(); i++) {
        hidden[i] = std::tanh(hiddenAcc[i]);
    }
    
    // Pass 2: hidden -> hidden/output
    std::vector<float> h2h(hidden.size(), 0.0f);
    
    for (auto& g : genes) {
        if (g.sourceType != 1) continue;
        
        float val = (g.sourceIndex < (int)hidden.size()) ? hidden[g.sourceIndex] : 0.0f;
        float contrib = val * g.weight;
        
        if (g.destType == 0 && g.destIndex < (int)h2h.size()) {
            h2h[g.destIndex] += contrib;
        } else if (g.destType == 1 && g.destIndex < (int)outputAcc.size()) {
            outputAcc[g.destIndex] += contrib;
        }
    }
    
    for (size_t i = 0; i < hidden.size(); i++) {
        hidden[i] = std::tanh(hidden[i] + h2h[i]);
    }
    
    for (size_t i = 0; i < output.size(); i++) {
        output[i] = std::tanh(outputAcc[i]);
    }
}

float Brain::getOutput(int index) const {
    return (index >= 0 && index < (int)output.size()) ? output[index] : 0.0f;
}

std::vector<uint32_t> Brain::randomGenome(int numGenes, std::mt19937& rng) {
    std::vector<uint32_t> genome;
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    
    genome.reserve(numGenes);
    for (int i = 0; i < numGenes; i++) {
        genome.push_back(dist(rng));
    }
    return genome;
}

std::vector<uint32_t> Brain::mutate(std::vector<uint32_t> genome,
                                     float rate, float strength,
                                     std::mt19937& rng,
                                     std::vector<uint32_t>* mutatedBits) {
    std::bernoulli_distribution shouldMutate(rate);
    std::uniform_int_distribution<int> bitDist(0, 31);
    
    int maxFlips = std::max(1, (int)(strength * 4));
    std::uniform_int_distribution<int> flipDist(1, maxFlips);
    
    if (mutatedBits) {
        mutatedBits->resize(genome.size(), 0);
    }
    
    for (size_t i = 0; i < genome.size(); i++) {
        if (shouldMutate(rng)) {
            int flips = flipDist(rng);
            for (int f = 0; f < flips; f++) {
                int bit = bitDist(rng);
                genome[i] ^= (1u << bit);
                if (mutatedBits) {
                    (*mutatedBits)[i] |= (1u << bit);
                }
            }
        }
    }
    return genome;
}

std::vector<uint32_t> Brain::crossover(const std::vector<uint32_t>& a,
                                        const std::vector<uint32_t>& b,
                                        std::mt19937& rng) {
    std::vector<uint32_t> child;
    size_t size = std::min(a.size(), b.size());
    std::bernoulli_distribution coin(0.5);
    
    child.reserve(size);
    for (size_t i = 0; i < size; i++) {
        child.push_back(coin(rng) ? a[i] : b[i]);
    }
    return child;
}
