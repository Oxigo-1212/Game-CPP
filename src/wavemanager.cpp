#include "include/WaveManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>

WaveManager::WaveManager()
    : currentWave(0),                                      // Start at wave 0 (game begins with wave 1)
      zombiesRemaining(0),                                 // No zombies initially
      zombiesToSpawn(0),                                   // No zombies to spawn initially
      spawnDelay(WaveConfig::INITIAL_SPAWN_DELAY),         // Initial time between spawns
      spawnTimer(0),                                       // Timer for next spawn
      waveDelayTimer(0),                                   // Timer for delay between waves
      waitingForNextWave(true),                            // Start in between-wave state
      newWeaponUnlocked(false),                            // No weapons unlocked initially
      zombiesInCurrentGroup(0),                            // No zombies in current group
      currentGroupSize(0),                                 // No group size initially
      lastTimerUpdate(0) {}                                // Track last timer update


void WaveManager::StartNextWave() {
    currentWave++;                       
    CalculateWaveParameters();           // Determine difficulty for this wave
    CheckWeaponUnlocks();                
    zombiesRemaining = zombiesToSpawn;   // Set zombies remaining to total for this wave
    spawnTimer = spawnDelay;             // Reset spawn timer
    waitingForNextWave = false;          // No longer waiting for next wave
    zombiesInCurrentGroup = 0;           // Reset group counter
    currentGroupSize = GetRandomGroupSize(); // Determine first group size
    lastTimerUpdate = spawnTimer;        // Track spawn timer for UI updates
}


 
void WaveManager::Update(float deltaTime) {
    // Handle the waiting period between waves
    if (waitingForNextWave) {
        waveDelayTimer -= deltaTime;      // Count down the wave delay timer
        if (waveDelayTimer <= 0) {
            StartNextWave();              // Start next wave when timer expires
        }
        return;
    }

    if (zombiesRemaining > 0) {
        // If current group has been fully spawned, prepare for next group
        if (zombiesInCurrentGroup >= currentGroupSize) {
            zombiesInCurrentGroup = 0;                  // Reset the counter for zombies in group
            currentGroupSize = GetRandomGroupSize();    // Calculate next group size
            spawnTimer = spawnDelay;                    // Reset the spawn timer
            lastTimerUpdate = spawnTimer;               // Update timer tracking
        }

        // Count down the time until next zombie group spawn
        spawnTimer -= deltaTime;
    }

    // Check if wave is complete (all zombies spawned and defeated)
    if (zombiesRemaining <= 0 && !waitingForNextWave) {
        waitingForNextWave = true;                      // Enter wave delay state
        waveDelayTimer = WaveConfig::WAVE_DELAY;        // Set the delay timer for next wave
        
        // Display wave completion message with ASCII art border
        std::cout << "\n+----------- Wave Complete -----------+" << std::endl;
        std::cout << "| Wave " << std::setw(2) << currentWave << " completed!                  |" << std::endl;
        std::cout << "| Next wave in " << std::fixed << std::setprecision(1) << std::setw(4) 
                 << WaveConfig::WAVE_DELAY << " seconds...        |" << std::endl;
        std::cout << "+--------------------------------+" << std::endl;
    }
}


bool WaveManager::ShouldSpawnZombie() const {
    // Only return true once per group when timer hits zero
    return !waitingForNextWave && zombiesRemaining > 0 && 
           spawnTimer <= 0 && zombiesInCurrentGroup == 0;
}


bool WaveManager::IsWaveComplete() const {
    return waitingForNextWave && zombiesRemaining == 0;
}

void WaveManager::OnZombieSpawned() {
    if (zombiesRemaining > 0) {
        zombiesRemaining--;         // One less zombie to spawn in this wave
        zombiesInCurrentGroup++;    // One more zombie in the current group
    }
}

void WaveManager::CheckWeaponUnlocks() {
    if (currentWave == WaveConfig::RIFLE_UNLOCK_WAVE || 
        currentWave == WaveConfig::SHOTGUN_UNLOCK_WAVE) {
        newWeaponUnlocked = true;
    }
}

bool WaveManager::HasNewWeaponUnlock() const {
    return newWeaponUnlocked;
}


void WaveManager::CalculateWaveParameters() {
    // Calculate number of zombies for this wave (base + increment per wave)
    zombiesToSpawn = WaveConfig::BASE_ZOMBIES_PER_WAVE + (currentWave - 1) * WaveConfig::ZOMBIES_INCREASE_PER_WAVE;
    
    // Cap the maximum zombies per wave to prevent overwhelming the player
    zombiesToSpawn = std::min(zombiesToSpawn, WaveConfig::MAX_ZOMBIES_PER_WAVE);
    
    // Calculate spawn delay, decreasing with each wave (faster spawns = harder waves)
    // Use std::max to ensure spawn delay never goes below minimum threshold
    spawnDelay = std::max(WaveConfig::MIN_SPAWN_DELAY,WaveConfig::INITIAL_SPAWN_DELAY - (currentWave - 1) *WaveConfig::SPAWN_DELAY_DECREASE);

    // Special handling for boss waves
    if (IsBossWave()) {
        // Reduce number of zombies in boss waves but make them stronger
        // (boss waves focus on quality over quantity)
        zombiesToSpawn = std::max(1, zombiesToSpawn / 3);
    }
}


int WaveManager::GetRandomGroupSize() const {
    // If no zombies remain, return 0
    if (zombiesRemaining <= 0) {
        return 0;
    }

    // Calculate bounds for the group size based on config values and remaining zombies
    int maxPossible = std::min(WaveConfig::MAX_GROUP_SIZE, zombiesRemaining);
    int minPossible = std::min(WaveConfig::MIN_GROUP_SIZE, zombiesRemaining);
    
    // If few zombies remaining, just spawn all of them
    if (zombiesRemaining <= minPossible) {
        return zombiesRemaining;
    }
    
    // Random group size between min and max
    // Use rand() to get a random number in the range [minPossible, maxPossible]
    return minPossible + (rand() % (maxPossible - minPossible + 1));
}


float WaveManager::GetHealthMultiplier() const {
    // Calculate base scaling multiplier (increases linearly with wave number)
    float multiplier = 1.0f + (currentWave - 1) * WaveConfig::HEALTH_INCREASE_PER_WAVE;
    
    // Apply special multiplier for boss waves
    if (IsBossWave()) {
        multiplier *= WaveConfig::BOSS_HEALTH_MULTIPLIER;
    }
    
    // Cap the health multiplier to prevent extreme difficulty
    return std::min(multiplier, static_cast<float>(WaveConfig::MAX_HEALTH_MULTIPLIER));
}


float WaveManager::GetSpeedMultiplier() const {
    // Calculate speed multiplier based on wave number
    float multiplier = 1.0f + (currentWave - 1) * WaveConfig::SPEED_INCREASE_PER_WAVE;
    
    // Cap the speed multiplier to keep movement reasonable
    return std::min(multiplier, WaveConfig::MAX_SPEED_MULTIPLIER);
}


float WaveManager::GetDamageMultiplier() const {
    float multiplier = 1.0f;
    
    // Only increase damage for boss waves
    if (IsBossWave()) {
        multiplier *= WaveConfig::BOSS_DAMAGE_MULTIPLIER;
    }
    
    return multiplier;
}