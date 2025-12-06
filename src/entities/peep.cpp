#include "peep.hpp"
#include "../core/constants.hpp"
#include <cmath>

Peep::Peep()
    : position(0, 0)
    , velocity(0, 0)
    , hunger(100.0f)
    , age(0.0f)
    , alive(true)
    , generation(0)
    , colorR(255)
    , colorG(255)
    , colorB(255) {}

void Peep::init(int numHidden, int numGenes) {
    brain.init(numHidden, numGenes);
    brain.setGenome(genome);
}

void Peep::updateColor() {
    const auto& genes = brain.getGenes();
    
    float foodInf = 0.0f, zoneInf = 0.0f, posInf = 0.0f, randInf = 0.0f;
    float totalWeight = 0.0f;
    
    for (auto& g : genes) {
        float w = std::abs(g.weight);
        
        if (g.sourceType == 0) {
            float inf = w * (g.destType == 1 ? 2.0f : 1.0f);
            int src = g.sourceIndex;
            
            if (src >= NeuralConfig::IN_FOOD_DX && src <= NeuralConfig::IN_HUNGER) {
                foodInf += inf;
            } else if (src >= NeuralConfig::IN_ZONE_DX && src <= NeuralConfig::IN_TIME_LEFT) {
                zoneInf += inf;
            } else if (src <= NeuralConfig::IN_WALL_DIST) {
                posInf += inf;
            } else if (src >= NeuralConfig::IN_RANDOM) {
                randInf += inf;
            }
        }
        totalWeight += w;
    }
    
    float total = foodInf + zoneInf + posInf + randInf;
    if (total < Constants::EPSILON) total = 1.0f;
    
    float foodR = foodInf / total;
    float zoneR = zoneInf / total;
    float posR = posInf / total;
    float randR = randInf / total;
    
    float hue = foodR * 30.0f + zoneR * 105.0f + posR * 195.0f + randR * 285.0f;
    
    uint32_t hash = 0;
    for (auto& g : genome) hash ^= (g >> 16);
    hue += (float)(hash % 40) - 20.0f;
    
    while (hue < 0) hue += 360.0f;
    while (hue >= 360.0f) hue -= 360.0f;
    
    float maxR = std::max({foodR, zoneR, posR, randR});
    float sat = 0.4f + maxR * 0.5f;
    
    float avgW = genes.empty() ? 0.0f : totalWeight / (float)genes.size();
    float lit = std::min(0.7f, 0.35f + avgW / 3.0f);
    
    float c = (1.0f - std::abs(2.0f * lit - 1.0f)) * sat;
    float h = hue / 60.0f;
    while (h >= 2.0f) h -= 2.0f;
    float x = c * (1.0f - std::abs(h - 1.0f));
    float m = lit - c / 2.0f;
    
    float r = 0, g = 0, b = 0;
    if (hue < 60) { r = c; g = x; }
    else if (hue < 120) { r = x; g = c; }
    else if (hue < 180) { g = c; b = x; }
    else if (hue < 240) { g = x; b = c; }
    else if (hue < 300) { r = x; b = c; }
    else { r = c; b = x; }
    
    colorR = (unsigned char)((r + m) * 255);
    colorG = (unsigned char)((g + m) * 255);
    colorB = (unsigned char)((b + m) * 255);
}
