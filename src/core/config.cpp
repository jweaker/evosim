#include "config.hpp"

void SimConfig::presetMigration() {
    resetToDefaults();
    initialPopulation = 200.0f;
    generationTime = 12.0f;
    genesPerPeep = 16.0f;
    hiddenNeurons = 4.0f;
    mutationRate = 0.12f;
    hungerEnabled = false;
    obstaclesEnabled = false;
    survivalZoneEnabled = true;
    survivalMode = 0.0f;
    spawnZonesEnabled = true;
    spawnOnLeft = true;
    peepSpeed = 120.0f;
    peepSenseRange = 100.0f;
}

void SimConfig::presetForaging() {
    resetToDefaults();
    initialPopulation = 100.0f;
    generationTime = 30.0f;
    genesPerPeep = 24.0f;
    hiddenNeurons = 6.0f;
    mutationRate = 0.08f;
    hungerEnabled = true;
    startingHunger = 60.0f;
    maxHunger = 150.0f;
    hungerDecayRate = 4.0f;
    foodCount = 60.0f;
    foodHunger = 35.0f;
    foodRespawnTime = 4.0f;
    obstaclesEnabled = false;
    survivalZoneEnabled = false;
    spawnZonesEnabled = false;
    spawnOnLeft = false;
    peepSpeed = 140.0f;
    peepSenseRange = 200.0f;
}

void SimConfig::presetMazeRunner() {
    resetToDefaults();
    initialPopulation = 250.0f;
    generationTime = 15.0f;
    genesPerPeep = 32.0f;
    hiddenNeurons = 8.0f;
    mutationRate = 0.15f;
    hungerEnabled = false;
    obstaclesEnabled = true;
    survivalZoneEnabled = true;
    survivalMode = 0.0f;
    spawnZonesEnabled = true;
    spawnOnLeft = true;
    peepSpeed = 100.0f;
    peepSenseRange = 80.0f;
}

void SimConfig::presetHungerGames() {
    resetToDefaults();
    initialPopulation = 150.0f;
    generationTime = 40.0f;
    genesPerPeep = 28.0f;
    hiddenNeurons = 8.0f;
    mutationRate = 0.10f;
    hungerEnabled = true;
    startingHunger = 50.0f;
    maxHunger = 120.0f;
    hungerDecayRate = 3.0f;
    foodCount = 25.0f;
    foodHunger = 60.0f;
    foodRespawnTime = 8.0f;
    obstaclesEnabled = true;
    survivalZoneEnabled = false;
    spawnZonesEnabled = false;
    spawnOnLeft = false;
    peepSpeed = 160.0f;
    peepSenseRange = 250.0f;
}

void SimConfig::presetBigBrains() {
    resetToDefaults();
    initialPopulation = 120.0f;
    generationTime = 20.0f;
    genesPerPeep = 64.0f;
    hiddenNeurons = 16.0f;
    mutationRate = 0.06f;
    useCrossover = true;
    crossoverRate = 0.9f;
    hungerEnabled = true;
    startingHunger = 80.0f;
    hungerDecayRate = 2.0f;
    foodCount = 50.0f;
    obstaclesEnabled = true;
    survivalZoneEnabled = true;
    survivalMode = 2.0f;
    spawnZonesEnabled = false;
    spawnOnLeft = false;
    peepSpeed = 100.0f;
    peepSenseRange = 180.0f;
}

void SimConfig::presetSpeedDemons() {
    resetToDefaults();
    initialPopulation = 300.0f;
    generationTime = 5.0f;
    genesPerPeep = 12.0f;
    hiddenNeurons = 3.0f;
    mutationRate = 0.20f;
    hungerEnabled = false;
    obstaclesEnabled = false;
    survivalZoneEnabled = true;
    survivalMode = 3.0f;
    spawnZonesEnabled = false;
    spawnOnLeft = false;
    peepSpeed = 300.0f;
    peepSenseRange = 150.0f;
}
