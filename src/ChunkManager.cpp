#include "include/ChunkManager.h"
#include <iostream>
#include <cmath>
#include <thread> // Ensure thread is included for std::this_thread::sleep_for
#include <chrono> // Ensure chrono is included for std::chrono::milliseconds

ChunkManager::ChunkManager(SDL_Renderer* renderer, Player* player, const std::string& baseMapPath, const std::string& baseTilesetPath)
    : renderer(renderer), player(player), baseMapPath(baseMapPath), baseTilesetPath(baseTilesetPath),
      blueprintTileMap(nullptr), currentPlayerChunkCoord({0,0}), 
      chunkWidthPixels(0), chunkHeightPixels(0), viewDistanceChunks(1) { // Default view distance to 1 chunk around player

    // Create a blueprint tilemap to get dimensions and for loading new chunks
    blueprintTileMap = new TileMap(renderer);
    if (!blueprintTileMap->LoadTileset(baseTilesetPath.c_str())) {
        std::cerr << "ChunkManager: Failed to load blueprint tileset: " << baseTilesetPath << std::endl;
    }
    if (!blueprintTileMap->LoadMap(baseMapPath.c_str())) {
        std::cerr << "ChunkManager: Failed to load blueprint map: " << baseMapPath << std::endl;
    }

    if (blueprintTileMap) {
        chunkWidthPixels = blueprintTileMap->GetPixelWidth();
        chunkHeightPixels = blueprintTileMap->GetPixelHeight();
    }

    if (chunkWidthPixels == 0 || chunkHeightPixels == 0) {
        std::cerr << "ChunkManager: Blueprint map has zero dimensions!" << std::endl;
    }
    
    UpdateActiveChunks(); 
}

ChunkManager::~ChunkManager() {
    // Wait for any pending loading tasks to complete
    for (auto& task_pair : loadingTasks) {
        if (task_pair.second.valid()) {
            task_pair.second.wait(); // Wair the background thread complete
        }
    }
    loadingTasks.clear();

    // Clear any chunks that were loaded but not yet activated
    {
        std::lock_guard<std::mutex> lock(readyChunksMutex); //Lock the mutex to safely access readyToActivateChunks without data race
        for (auto& pair : readyToActivateChunks) {
            delete pair.second; // Delete TileMap instance
        }
        readyToActivateChunks.clear();
    }

    for (auto& pair : activeChunks) {
        delete pair.second; // Delete TileMap instance
    }
    activeChunks.clear();

    if (blueprintTileMap) {
        delete blueprintTileMap;
        blueprintTileMap = nullptr;
    }
}

ChunkCoord ChunkManager::GetChunkCoordFromWorldPos(float worldX, float worldY) {
    if (chunkWidthPixels == 0 || chunkHeightPixels == 0) {
        return {0,0};
    }
    int chunkX = static_cast<int>(std::floor(worldX / chunkWidthPixels));
    int chunkY = static_cast<int>(std::floor(worldY / chunkHeightPixels));
    return {chunkX, chunkY}; // Chunk coordinates
}

void ChunkManager::LoadChunk(int chunkGridX, int chunkGridY) {
    ChunkCoord coord = {chunkGridX, chunkGridY};

    if (activeChunks.count(coord) || loadingTasks.count(coord)) {
        return;
    }

    if (!blueprintTileMap || chunkWidthPixels == 0 || chunkHeightPixels == 0) {
        std::cerr << "ChunkManager: Cannot start load chunk, blueprint not ready." << std::endl;
        return;
    }
    //Force run on a separate thread
    // This is a simple example of using std::async to run the loading in a separate thread.
    loadingTasks[coord] = std::async(std::launch::async, [this, chunkGridX, chunkGridY, coord]() {
        TileMap* newChunk = new TileMap(this->renderer); 

        

        bool success = true;
        // SDL_CreateTextureFromSurface should ideally be on the main thread.
        // For this example, we are calling it here. If issues arise, this is a key area to refactor.
        if (!newChunk->LoadTileset(this->baseTilesetPath.c_str())) {
            std::cerr << "Thread: Failed to load tileset for chunk (" << chunkGridX << "," << chunkGridY << ")" << std::endl;
            success = false;
        }
        if (success && !newChunk->LoadMap(this->baseMapPath.c_str())) {
            std::cerr << "Thread: Failed to load map data for chunk (" << chunkGridX << "," << chunkGridY << ")" << std::endl;
            success = false;
        }

        if (success) {
            std::lock_guard<std::mutex> lock(this->readyChunksMutex);
            this->readyToActivateChunks.push_back({coord, newChunk});
        } else {
            delete newChunk; 
        }
    });
}

void ChunkManager::UnloadChunk(int chunkGridX, int chunkGridY) {
    ChunkCoord coord = {chunkGridX, chunkGridY};
    auto it = activeChunks.find(coord);
    if (it != activeChunks.end()) {
        delete it->second; 
        activeChunks.erase(it);
    }
}

void ChunkManager::ProcessReadyChunks() {
    std::vector<std::pair<ChunkCoord, TileMap*>> toActivateNow;
    {
        std::lock_guard<std::mutex> lock(readyChunksMutex);
        if (!readyToActivateChunks.empty()) {
            toActivateNow.swap(readyToActivateChunks);
        }
    }

    for (const auto& pair : toActivateNow) {
        ChunkCoord coord = pair.first;
        TileMap* chunk = pair.second;
        loadingTasks.erase(coord); // Remove from loading tasks map once processed, regardless of outcome

        bool stillNeeded = false;
        for (int yOffset = -viewDistanceChunks; yOffset <= viewDistanceChunks; ++yOffset) {
            for (int xOffset = -viewDistanceChunks; xOffset <= viewDistanceChunks; ++xOffset) {
                if (currentPlayerChunkCoord.x + xOffset == coord.x && currentPlayerChunkCoord.y + yOffset == coord.y) {
                    stillNeeded = true;
                    break;
                }
            }
            if (stillNeeded) break;
        }

        if (stillNeeded) {
            if (activeChunks.find(coord) == activeChunks.end()) { 
                activeChunks[coord] = chunk;
            } else {
                delete chunk; 
            }
        } else {
            delete chunk;
        }
    }

    auto it_futures = loadingTasks.begin();
    while (it_futures != loadingTasks.end()) {
        if (it_futures->second.valid() && 
            it_futures->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            // Future is ready, meaning the task finished (or threw an exception handled by std::async)
            // We attempt to get the result to clear the future's state and catch exceptions if any were propagated.
            try {
                it_futures->second.get(); 
            } catch (const std::exception& e) {
                std::cerr << "Exception from loading task for chunk (" << it_futures->first.x << "," << it_futures->first.y << "): " << e.what() << std::endl;
            }
            it_futures = loadingTasks.erase(it_futures); 
        } else {
            ++it_futures;
        }
    }
}

void ChunkManager::UpdateActiveChunks() {
    if (!player || chunkWidthPixels == 0 || chunkHeightPixels == 0) return;

    ChunkCoord newPlayerChunkCoord = GetChunkCoordFromWorldPos(player->GetX(), player->GetY());

    if (newPlayerChunkCoord.x != currentPlayerChunkCoord.x || 
        newPlayerChunkCoord.y != currentPlayerChunkCoord.y || 
        activeChunks.empty()) {
        
        currentPlayerChunkCoord = newPlayerChunkCoord;

        std::map<ChunkCoord, bool> requiredChunks;
        for (int yOffset = -viewDistanceChunks; yOffset <= viewDistanceChunks; ++yOffset) {
            for (int xOffset = -viewDistanceChunks; xOffset <= viewDistanceChunks; ++xOffset) {
                requiredChunks[{currentPlayerChunkCoord.x + xOffset, currentPlayerChunkCoord.y + yOffset}] = true;
            }
        }

        std::vector<ChunkCoord> toUnload;
        for (const auto& pair : activeChunks) {
            if (requiredChunks.find(pair.first) == requiredChunks.end()) {
                toUnload.push_back(pair.first);
            }
        }
        for (const auto& coord : toUnload) {
            UnloadChunk(coord.x, coord.y);
        }

        for (const auto& pair : requiredChunks) {
            // Only attempt to load if not active AND not already being loaded
            if (activeChunks.find(pair.first) == activeChunks.end() && loadingTasks.find(pair.first) == loadingTasks.end()) {
                LoadChunk(pair.first.x, pair.first.y); 
            }
        }
    }
}

void ChunkManager::Update(float deltaTime) {
    ProcessReadyChunks();
    UpdateActiveChunks();
}

void ChunkManager::Render(Camera* camera) {
    if (!camera || chunkWidthPixels == 0 || chunkHeightPixels == 0) return;

    for (const auto& pair : activeChunks) {
        ChunkCoord coord = pair.first;
        TileMap* chunk = pair.second;
        // Convert chunk coordinates to world coordinates
        int worldOffsetX = coord.x * chunkWidthPixels;
        int worldOffsetY = coord.y * chunkHeightPixels;

        chunk->Render(camera, worldOffsetX, worldOffsetY);
    }
}
