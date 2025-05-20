#pragma once

namespace WaveConfig {
    // Base wave settings
    static constexpr int BASE_ZOMBIES_PER_WAVE = 5;
    static constexpr int ZOMBIES_INCREASE_PER_WAVE = 2;  // How many more zombies each wave
    static constexpr int MAX_ZOMBIES_PER_WAVE = 50;     // Cap on zombies per wave
      // Base zombie stats
    static constexpr int ZOMBIE_BASE_HEALTH = 5;         // Starting health points
    static constexpr float ZOMBIE_BASE_SPEED = 100.0f;   // Base movement speed
    static constexpr int ZOMBIE_BASE_DAMAGE = 10;        // Base damage per hit

    // Spawn timing and grouping
    static constexpr float INITIAL_SPAWN_DELAY = 2.0f;   // Time between zombie groups in seconds
    static constexpr float SPAWN_DELAY_DECREASE = 0.1f;  // How much to decrease spawn delay each wave
    static constexpr float MIN_SPAWN_DELAY = 0.5f;       // Minimum time between group spawns
    static constexpr float WAVE_DELAY = 10.0f;           // Time between waves
    static constexpr int MIN_GROUP_SIZE = 5;             // Minimum zombies per spawn group
    static constexpr int MAX_GROUP_SIZE = 8;             // Maximum zombies per spawn group
    static constexpr float GROUP_SPAWN_RADIUS = 100.0f;  // Radius within which group members spawn
    static constexpr int SPAWN_POINTS = 4;               // Number of different spawn points around player
    
    // Horde behavior
    static constexpr float NEIGHBOR_RADIUS = 100.0f;     // Distance for zombie grouping behavior
    static constexpr float SEPARATION_WEIGHT = 1.5f;     // Weight for keeping distance between zombies
    static constexpr float COHESION_WEIGHT = 1.0f;       // Weight for moving toward zombie center
    static constexpr float ALIGNMENT_WEIGHT = 1.0f;      // Weight for moving in same direction
    static constexpr float SPEED_VARIATION = 0.2f;       // +/- % variation in zombie speed

    // Weapon unlock waves
    static constexpr int RIFLE_UNLOCK_WAVE = 3;          // Rifle unlocks at wave 3
    static constexpr int SHOTGUN_UNLOCK_WAVE = 5;        // Shotgun unlocks at wave 5
    
    // Spawn positioning
    static constexpr float MIN_SPAWN_DISTANCE = 400.0f;  // Minimum distance from player to spawn
    static constexpr float MAX_SPAWN_DISTANCE = 800.0f;  // Maximum distance from player to spawn
    
    // Zombie scaling per wave
    static constexpr float HEALTH_INCREASE_PER_WAVE = 0.2f;  // 20% more health each wave
    static constexpr float SPEED_INCREASE_PER_WAVE = 0.1f;   // 10% more speed each wave
    static constexpr int MAX_HEALTH_MULTIPLIER = 5;          // Cap health scaling at 5x
    static constexpr float MAX_SPEED_MULTIPLIER = 2.0f;      // Cap speed scaling at 2x
    
    // Special waves
    static constexpr int BOSS_WAVE_INTERVAL = 5;         // Boss wave every X waves
    static constexpr float BOSS_HEALTH_MULTIPLIER = 5.0f;// Boss has 5x health
    static constexpr float BOSS_DAMAGE_MULTIPLIER = 2.0f;// Boss does 2x damage
}
