#include "spawn_zone.hpp"

sf::Vector2f SpawnZone::randomPoint(std::mt19937& rng) const {
    std::uniform_real_distribution<float> dx(pos.x, pos.x + sz.x);
    std::uniform_real_distribution<float> dy(pos.y, pos.y + sz.y);
    return {dx(rng), dy(rng)};
}
