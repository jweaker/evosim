#pragma once
#include <cstdint>
#include <string>

struct Gene {
    int sourceType;
    int sourceIndex;
    int destType;
    int destIndex;
    float weight;
    
    uint32_t encode() const;
    static Gene decode(uint32_t encoded);
    static std::string toBinaryString(uint32_t encoded);
    static std::string toDisplayString(uint32_t encoded);
};
