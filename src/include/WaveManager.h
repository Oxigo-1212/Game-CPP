#pragma once
#include "WaveConfig.h"
#include <cstdint>

class WaveManager {
public:
    WaveManager();
    
    // Wave control
    void StartNextWave();
    void Update(float deltaTime);
    bool ShouldSpawnZombie() const;
    bool IsWaveComplete() const;
    
    // Wave state getters
    int GetCurrentWave() const { return currentWave; }
    int GetZombiesRemaining() const { return zombiesRemaining; }
    float GetSpawnDelay() const { return spawnTimer > 0 ? spawnTimer : 0.0f; }
    float GetWaveDelay() const { return waitingForNextWave ? waveDelayTimer : 0.0f; }
    int GetCurrentGroupSize() const { return currentGroupSize; }
    bool IsBossWave() const { return currentWave % WaveConfig::BOSS_WAVE_INTERVAL == 0; }
    
    // Weapon unlocks
    bool IsRifleUnlocked() const { return currentWave >= WaveConfig::RIFLE_UNLOCK_WAVE; }
    bool IsShotgunUnlocked() const { return currentWave >= WaveConfig::SHOTGUN_UNLOCK_WAVE; }
    bool HasNewWeaponUnlock() const;
    void AcknowledgeWeaponUnlock() { newWeaponUnlocked = false; }
    
    // Wave scaling getters
    float GetHealthMultiplier() const;
    float GetSpeedMultiplier() const;    float GetDamageMultiplier() const;
    void OnZombieSpawned();
    
private:
    // Get random size for next zombie group
    int GetRandomGroupSize() const;

    int currentWave;
    int zombiesRemaining;
    int zombiesToSpawn;
    float spawnDelay;
    float spawnTimer;
    float waveDelayTimer;
    bool waitingForNextWave;
    bool newWeaponUnlocked;
      // Group spawning state
    int zombiesInCurrentGroup;  // How many zombies spawned in current group
    int currentGroupSize;       // Total size of current group
    float lastTimerUpdate;      // For tracking spawn timer changes

    void CalculateWaveParameters();
    void CheckWeaponUnlocks();
};