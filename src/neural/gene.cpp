#include "gene.hpp"
#include "../core/constants.hpp"
#include <sstream>
#include <iomanip>

namespace {
    const float WEIGHT_RANGE = 8.0f;
    const float WEIGHT_OFFSET = 4.0f;
    const float WEIGHT_MAX = 8388607.0f;
}

uint32_t Gene::encode() const {
    uint32_t encoded = 0;
    
    int source = (sourceType == 0) ? sourceIndex : 15 + sourceIndex;
    if (source < 0) source = 0;
    if (source > 31) source = 31;
    encoded |= ((uint32_t)source & 0x1F) << 27;
    
    int dest = (destType == 0) ? destIndex : 8 + destIndex;
    if (dest < 0) dest = 0;
    if (dest > 15) dest = 15;
    encoded |= ((uint32_t)dest & 0xF) << 23;
    
    float w = weight;
    if (w < -4.0f) w = -4.0f;
    if (w > 4.0f) w = 4.0f;
    
    float norm = (w + WEIGHT_OFFSET) / WEIGHT_RANGE;
    uint32_t bits = (uint32_t)(norm * WEIGHT_MAX);
    encoded |= (bits & 0x7FFFFF);
    
    return encoded;
}

Gene Gene::decode(uint32_t encoded) {
    Gene g;
    
    int source = (encoded >> 27) & 0x1F;
    if (source < 15) {
        g.sourceType = 0;
        g.sourceIndex = source;
    } else {
        g.sourceType = 1;
        g.sourceIndex = (source - 15) % 8;
    }
    
    int dest = (encoded >> 23) & 0xF;
    if (dest < 8) {
        g.destType = 0;
        g.destIndex = dest;
    } else {
        g.destType = 1;
        g.destIndex = (dest - 8) % NeuralConfig::OUTPUT_COUNT;
    }
    
    uint32_t bits = encoded & 0x7FFFFF;
    float norm = (float)bits / WEIGHT_MAX;
    g.weight = (norm * WEIGHT_RANGE) - WEIGHT_OFFSET;
    
    return g;
}

std::string Gene::toBinaryString(uint32_t encoded) {
    std::string result;
    for (int i = 31; i >= 0; i--) {
        result += ((encoded >> i) & 1) ? '1' : '0';
        if (i == 27 || i == 23) result += ' ';
    }
    return result;
}

std::string Gene::toDisplayString(uint32_t encoded) {
    Gene g = decode(encoded);
    std::ostringstream ss;
    
    if (g.sourceType == 0) {
        const char* names[] = {
            "PosX", "PosY", "FdDX", "FdDY", "FdDs",
            "Wall", "Hngr", "Age", "Time", "ZnDX",
            "ZnDY", "ZnDs", "InZn", "Rand", "Bias"
        };
        ss << (g.sourceIndex < 15 ? names[g.sourceIndex] : "In?");
    } else {
        ss << "H" << g.sourceIndex;
    }
    
    ss << " --[" << std::fixed << std::setprecision(2) << g.weight << "]--> ";
    
    if (g.destType == 0) {
        ss << "H" << g.destIndex;
    } else {
        ss << (g.destIndex == 0 ? "MoveX" : "MoveY");
    }
    
    return ss.str();
}
