#pragma once

#include <vector>
#include <map>
#include <string>
#include <thread> // Added
#include <mutex>  // Added
#include <future> // Added
#include "TileMap.h"
#include "Player.h"
#include "Camera.h"

// Define a simple struct or pair for chunk coordinates
struct ChunkCoord {
    int x, y;
    // Comparison operator for using ChunkCoord as a map key
    bool operator<(const ChunkCoord& other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        return y < other.y;
    }
    

};

class ChunkManager {
public:
    ChunkManager(SDL_Renderer* renderer, Player* player, const std::string& baseMapPath, const std::string& baseTilesetPath);
    ~ChunkManager();

    void Update(float deltaTime);
    void Render(Camera* camera);

private:
    SDL_Renderer* renderer;
    Player* player;
    std::string baseMapPath;
    std::string baseTilesetPath;
    
    TileMap* blueprintTileMap; // Used to get dimensions and as a blueprint

    std::map<ChunkCoord, TileMap*> activeChunks;
    ChunkCoord currentPlayerChunkCoord;

    int chunkWidthPixels;
    int chunkHeightPixels;
    
    int viewDistanceChunks; // e.g., 1 means a 3x3 grid (player's chunk +/- 1)

    // --- Asynchronous Loading Members ---
    std::map<ChunkCoord, std::future<void>> loadingTasks; // Tracks active loading operations
    std::vector<std::pair<ChunkCoord, TileMap*>> readyToActivateChunks; // Chunks loaded by threads, awaiting activation
    std::mutex readyChunksMutex; // Mutex to protect readyToActivateChunks

    void LoadChunk(int chunkGridX, int chunkGridY); // Will be made async
    void UnloadChunk(int chunkGridX, int chunkGridY);
    ChunkCoord GetChunkCoordFromWorldPos(float worldX, float worldY);
    void UpdateActiveChunks();
    void ProcessReadyChunks(); // New method to move loaded chunks to activeChunks
};
