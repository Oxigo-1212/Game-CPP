#include "include/WaveManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>

WaveManager::WaveManager()
    : currentWave(0), zombiesRemaining(0), zombiesToSpawn(0),
      spawnDelay(WaveConfig::INITIAL_SPAWN_DELAY), spawnTimer(0),
      waveDelayTimer(0), waitingForNextWave(true), newWeaponUnlocked(false),
      zombiesInCurrentGroup(0), currentGroupSize(0), lastTimerUpdate(0) {}

void WaveManager::StartNextWave() {
    currentWave++;
    CalculateWaveParameters();
    CheckWeaponUnlocks();
    zombiesRemaining = zombiesToSpawn;
    spawnTimer = spawnDelay;
    waitingForNextWave = false;
    zombiesInCurrentGroup = 0;
    currentGroupSize = GetRandomGroupSize();
    lastTimerUpdate = spawnTimer;

    std::cout << "\n+================================+" << std::endl;
    std::cout << "|          WAVE " << std::setw(2) << currentWave << " STARTING          |" << std::endl;
    std::cout << "+================================+" << std::endl;
    std::cout << "| Total Zombies: " << std::setw(3) << zombiesToSpawn << "               |" << std::endl;
    std::cout << "| Group Size: " << std::setw(3) << currentGroupSize << "                 |" << std::endl;
    std::cout << "| Spawn Delay: " << std::fixed << std::setprecision(1) << std::setw(4) << spawnDelay << "s              |" << std::endl;
    if (IsBossWave()) {
        std::cout << "| !!! BOSS WAVE !!!                 |" << std::endl;
    }
    std::cout << "+================================+" << std::endl;
}

void WaveManager::Update(float deltaTime) {
    if (waitingForNextWave) {
        waveDelayTimer -= deltaTime;
        if (waveDelayTimer <= 0) {
            StartNextWave();
        }
        return;
    }

    if (zombiesRemaining > 0) {
        // If we need a new group
        if (zombiesInCurrentGroup >= currentGroupSize) {
            zombiesInCurrentGroup = 0;
            currentGroupSize = GetRandomGroupSize();
            spawnTimer = spawnDelay;
            lastTimerUpdate = spawnTimer;
            std::cout << "\n+------------ Next Group ------------+" << std::endl;
            std::cout << "| Size: " << std::setw(2) << currentGroupSize 
                     << " zombies  |  Remaining: " << std::setw(3) << zombiesRemaining << "  |" << std::endl;
            std::cout << "+--------------------------------+" << std::endl;
        }

        // Update spawn timer
        spawnTimer -= deltaTime;
        
        // Only show timer update when it changes by at least 0.1 seconds
        if (std::abs(spawnTimer - lastTimerUpdate) >= 0.1f) {
            std::cout << "\rSpawning in: " << std::fixed << std::setprecision(1) 
                     << std::setw(4) << std::max(0.0f, spawnTimer) << "s | "
                     << "Wave " << currentWave << " | "
                     << "Remaining: " << std::setw(3) << zombiesRemaining << " " << std::flush;
            lastTimerUpdate = spawnTimer;
        }
    }

    if (zombiesRemaining <= 0 && !waitingForNextWave) {
        waitingForNextWave = true;
        waveDelayTimer = WaveConfig::WAVE_DELAY;
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
    return waitingForNextWave && zombiesRemaining <= 0;
}

void WaveManager::OnZombieSpawned() {
    if (zombiesRemaining > 0) {
        zombiesRemaining--;
        zombiesInCurrentGroup++;
        
        // Print status when the entire group has spawned
        if (zombiesInCurrentGroup >= currentGroupSize) {
            float healthMult = GetHealthMultiplier();
            float speedMult = GetSpeedMultiplier();
            float damageMult = GetDamageMultiplier();
            
            std::cout << "\n+----------- Group Spawned -----------+" << std::endl;
            std::cout << "| Spawned: " << std::setw(2) << currentGroupSize 
                     << " zombies                  |" << std::endl;
            std::cout << "| Health: x" << std::fixed << std::setprecision(1) << std::setw(3) << healthMult 
                     << "  Speed: x" << std::setw(3) << speedMult 
                     << "  Damage: x" << std::setw(3) << damageMult << " |" << std::endl;
            std::cout << "+--------------------------------+" << std::endl;
            
            if (zombiesRemaining > 0) {
                int nextGroupSize = std::min(WaveConfig::MAX_GROUP_SIZE, zombiesRemaining);
                std::cout << "Next group will have " << nextGroupSize << " zombies" << std::endl;
            }
        }
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
    zombiesToSpawn = WaveConfig::BASE_ZOMBIES_PER_WAVE + 
                     (currentWave - 1) * WaveConfig::ZOMBIES_INCREASE_PER_WAVE;
    zombiesToSpawn = std::min(zombiesToSpawn, WaveConfig::MAX_ZOMBIES_PER_WAVE);
    
    spawnDelay = std::max(
        WaveConfig::MIN_SPAWN_DELAY,
        WaveConfig::INITIAL_SPAWN_DELAY - (currentWave - 1) * WaveConfig::SPAWN_DELAY_DECREASE
    );

    if (IsBossWave()) {
        zombiesToSpawn = std::max(1, zombiesToSpawn / 3);
    }
}

int WaveManager::GetRandomGroupSize() const {
    if (zombiesRemaining <= 0) {
        return 0;
    }

    int maxPossible = std::min(WaveConfig::MAX_GROUP_SIZE, zombiesRemaining);
    int minPossible = std::min(WaveConfig::MIN_GROUP_SIZE, zombiesRemaining);
    
    if (zombiesRemaining <= minPossible) {
        return zombiesRemaining;
    }
    
    return minPossible + (rand() % (maxPossible - minPossible + 1));
}

float WaveManager::GetHealthMultiplier() const {
    float multiplier = 1.0f + (currentWave - 1) * WaveConfig::HEALTH_INCREASE_PER_WAVE;
    
    if (IsBossWave()) {
        multiplier *= WaveConfig::BOSS_HEALTH_MULTIPLIER;
    }
    
    return std::min(multiplier, static_cast<float>(WaveConfig::MAX_HEALTH_MULTIPLIER));
}

float WaveManager::GetSpeedMultiplier() const {
    float multiplier = 1.0f + (currentWave - 1) * WaveConfig::SPEED_INCREASE_PER_WAVE;
    return std::min(multiplier, WaveConfig::MAX_SPEED_MULTIPLIER);
}

float WaveManager::GetDamageMultiplier() const {
    float multiplier = 1.0f;
    
    if (IsBossWave()) {
        multiplier *= WaveConfig::BOSS_DAMAGE_MULTIPLIER;
    }
    
    return multiplier;
}