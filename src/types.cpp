// types.cpp - Implementation of core data structures

#include "types.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ============================================================================
// GENE IMPLEMENTATION - Optimized for bit-flip mutations
// ============================================================================
//
// Bit layout (supports 15 inputs + 8 hidden neurons):
//   Bits 31-27 (5 bits): source neuron (0-14 = inputs, 15-22 = hidden 0-7)
//   Bits 26-23 (4 bits): dest neuron (0-7 = hidden, 8-15 = outputs, wraps)
//   Bits 22-0 (23 bits): weight (-4.0 to +4.0 range)
//
// This means bit flips have more predictable effects:
//   - High bits (27-31): change source neuron
//   - Mid bits (23-26): change destination neuron  
//   - Low bits (0-22): change weight (bit 22 = sign, lower = finer changes)

uint32_t Gene::encode() const {
    uint32_t encoded = 0;
    
    // Source: combine type and index into 5 bits (0-31)
    // Input neurons: 0-14, Hidden neurons: 15-22 (maps to hidden 0-7)
    int source = (sourceType == 0) ? sourceIndex : (15 + sourceIndex);
    source = std::max(0, std::min(31, source));
    encoded |= (static_cast<uint32_t>(source) & 0x1F) << 27;
    
    // Dest: combine type and index into 4 bits (0-15)
    // Hidden neurons: 0-7, Output neurons: 8-15 (maps to output 0-1)
    int dest = (destType == 0) ? destIndex : (8 + destIndex);
    dest = std::max(0, std::min(15, dest));
    encoded |= (static_cast<uint32_t>(dest) & 0xF) << 23;
    
    // Weight: map -4.0...+4.0 to 0...8388607 (23 bits)
    float clampedWeight = std::max(-4.0f, std::min(4.0f, weight));
    float normalized = (clampedWeight + 4.0f) / 8.0f;  // 0.0 to 1.0
    uint32_t weightBits = static_cast<uint32_t>(normalized * 8388607.0f);
    encoded |= (weightBits & 0x7FFFFF);
    
    return encoded;
}

Gene Gene::decode(uint32_t encoded) {
    Gene gene;
    
    // Extract source (5 bits)
    int source = (encoded >> 27) & 0x1F;
    if (source < 15) {
        gene.sourceType = 0;  // Input neuron
        gene.sourceIndex = source;
    } else {
        gene.sourceType = 1;  // Hidden neuron
        gene.sourceIndex = (source - 15) % 8;  // Wrap to 0-7
    }
    
    // Extract dest (4 bits)
    int dest = (encoded >> 23) & 0xF;
    if (dest < 8) {
        gene.destType = 0;  // Hidden neuron
        gene.destIndex = dest;
    } else {
        gene.destType = 1;  // Output neuron
        gene.destIndex = (dest - 8) % 2;  // Only 2 outputs, so wrap
    }
    
    // Extract weight (23 bits) and map to -4.0...+4.0
    uint32_t weightBits = encoded & 0x7FFFFF;
    float normalized = static_cast<float>(weightBits) / 8388607.0f;  // 0.0 to 1.0
    gene.weight = (normalized * 8.0f) - 4.0f;  // -4.0 to +4.0
    
    return gene;
}

std::string Gene::toBinaryString(uint32_t encoded) {
    std::string result;
    result.reserve(35);  // 32 bits + 3 spaces
    
    for (int i = 31; i >= 0; i--) {
        result += ((encoded >> i) & 1) ? '1' : '0';
        // Add separator between sections (5 bits source, 4 bits dest, 23 bits weight)
        if (i == 27 || i == 23) result += ' ';
    }
    
    return result;
}

std::string Gene::toDisplayString(uint32_t encoded) {
    Gene g = decode(encoded);
    std::ostringstream ss;
    
    // Source
    if (g.sourceType == 0) {
        const char* inputNames[] = {"PosX", "PosY", "FdDX", "FdDY", "FdDs", 
                                     "Wall", "Enrg", "Age", "Time", "ZnDX", 
                                     "ZnDY", "ZnDs", "InZn", "Rand", "Bias"};
        if (g.sourceIndex < 15) {
            ss << inputNames[g.sourceIndex];
        } else {
            ss << "In" << g.sourceIndex;
        }
    } else {
        ss << "H" << g.sourceIndex;
    }
    
    // Arrow with weight
    ss << " --[" << std::fixed << std::setprecision(2) << g.weight << "]--> ";
    
    // Dest
    if (g.destType == 0) {
        ss << "H" << g.destIndex;
    } else {
        ss << (g.destIndex == 0 ? "MoveX" : "MoveY");
    }
    
    return ss.str();
}

// ============================================================================
// PEEP IMPLEMENTATION
// ============================================================================

Peep::Peep() 
    : position(0.0f, 0.0f)
    , velocity(0.0f, 0.0f)
    , hunger(100.0f)
    , age(0.0f)
    , alive(true)
    , generation(0)
    , speciesId(0)
    , colorR(255)
    , colorG(255)
    , colorB(255) {
}

// ============================================================================
// FOOD IMPLEMENTATION
// ============================================================================

Food::Food() 
    : position(0.0f, 0.0f)
    , eaten(false)
    , respawnTimer(0.0f) {
}

Food::Food(float x, float y) 
    : position(x, y)
    , eaten(false)
    , respawnTimer(0.0f) {
}

// ============================================================================
// OBSTACLE IMPLEMENTATION
// ============================================================================

Obstacle::Obstacle() 
    : position(0.0f, 0.0f)
    , size(50.0f, 50.0f) {
}

Obstacle::Obstacle(float x, float y, float w, float h) 
    : position(x, y)
    , size(w, h) {
}

bool Obstacle::contains(sf::Vector2f point) const {
    return point.x >= position.x && 
           point.x <= position.x + size.x &&
           point.y >= position.y && 
           point.y <= position.y + size.y;
}

sf::Vector2f Obstacle::closestPoint(sf::Vector2f point) const {
    float closestX = std::max(position.x, std::min(point.x, position.x + size.x));
    float closestY = std::max(position.y, std::min(point.y, position.y + size.y));
    return sf::Vector2f(closestX, closestY);
}

// ============================================================================
// SURVIVAL ZONE IMPLEMENTATION
// ============================================================================

SurvivalZone::SurvivalZone() 
    : position(0.0f, 0.0f)
    , size(100.0f, 100.0f) {
}

SurvivalZone::SurvivalZone(float x, float y, float w, float h) 
    : position(x, y)
    , size(w, h) {
}

bool SurvivalZone::contains(sf::Vector2f point) const {
    return point.x >= position.x && 
           point.x <= position.x + size.x &&
           point.y >= position.y && 
           point.y <= position.y + size.y;
}

// ============================================================================
// SIMSTATS IMPLEMENTATION
// ============================================================================

SimStats::SimStats() 
    : currentGeneration(0)
    , populationCount(0)
    , aliveCount(0)
    , averageHunger(0.0f)
    , survivalRate(0.0f)
    , geneticDiversity(0.0f)
    , speciesCount(0) {
}

void SimStats::recordGeneration(float survival, int population, float avgHunger) {
    const size_t maxHistory = 100;
    
    survivalHistory.push_back(survival);
    populationHistory.push_back(static_cast<float>(population));
    energyHistory.push_back(avgHunger);
    
    if (survivalHistory.size() > maxHistory) {
        survivalHistory.erase(survivalHistory.begin());
        populationHistory.erase(populationHistory.begin());
        energyHistory.erase(energyHistory.begin());
    }
}
