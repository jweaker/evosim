#pragma once

namespace Constants {
    constexpr float EPSILON = 0.001f;
    constexpr float WORLD_MARGIN = 10.0f;
    constexpr float MIN_DRAWABLE_SIZE = 10.0f;
    constexpr int MAX_POSITION_ATTEMPTS = 100;
    constexpr int MAX_HISTORY_SIZE = 100;
    constexpr float MAX_PHYSICS_STEP = 0.1f;
    constexpr float MAX_FRAME_TIME = 0.1f;
}

namespace NeuralConfig {
    constexpr int INPUT_COUNT = 15;
    constexpr int OUTPUT_COUNT = 2;
    constexpr int MAX_HIDDEN = 32;

    // Input indices
    constexpr int IN_POS_X = 0;
    constexpr int IN_POS_Y = 1;
    constexpr int IN_FOOD_DX = 2;
    constexpr int IN_FOOD_DY = 3;
    constexpr int IN_FOOD_DIST = 4;
    constexpr int IN_WALL_DIST = 5;
    constexpr int IN_HUNGER = 6;
    constexpr int IN_AGE = 7;
    constexpr int IN_TIME_LEFT = 8;
    constexpr int IN_ZONE_DX = 9;
    constexpr int IN_ZONE_DY = 10;
    constexpr int IN_ZONE_DIST = 11;
    constexpr int IN_ZONE = 12;
    constexpr int IN_RANDOM = 13;
    constexpr int IN_BIAS = 14;

    // Output indices
    constexpr int OUT_MOVE_X = 0;
    constexpr int OUT_MOVE_Y = 1;
}
