#include "include/TileMap.h"
#include "include/Camera.h" // Include Camera for its definition
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath> // Added for std::round
#include <vector> // For std::vector, though already used for map
#include <algorithm> // For std::max, std::min

TileMap::TileMap(SDL_Renderer* renderer)
    : renderer(renderer), tileset(nullptr), tileWidth(32), tileHeight(32), 
      mapWidth(0), mapHeight(0), tilesetCols(0) {
}

TileMap::~TileMap() {
    if (tileset) {
        SDL_DestroyTexture(tileset);
        tileset = nullptr;
    }
}

bool TileMap::LoadTileset(const char* path) {
    if (tileset) {
        SDL_DestroyTexture(tileset);
        tileset = nullptr;
    }

    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load tileset image: " << IMG_GetError() << std::endl;
        return false;
    }

    tileset = SDL_CreateTextureFromSurface(renderer, surface);
    if (!tileset) {
        std::cerr << "Failed to create tileset texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }

    // Calculate number of columns in the tileset
    tilesetCols = surface->w / tileWidth;
    
    SDL_FreeSurface(surface);
    return true;
}

bool TileMap::ParseCSV(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << path << std::endl;
        return false;
    }

    map.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            try {
                row.push_back(std::stoi(cell));
            } catch (const std::exception& e) {
                std::cerr << "Error parsing tile ID: " << cell << std::endl;
                return false;
            }
        }

        if (row.size() > 0) {
            if (mapWidth == 0) {
                mapWidth = row.size();
            } else if (mapWidth != row.size()) {
                std::cerr << "Inconsistent map width in CSV file" << std::endl;
                return false;
            }
            map.push_back(row);
        }
    }

    mapHeight = map.size();
    return true;
}

bool TileMap::LoadMap(const char* path) {
    return ParseCSV(path);
}

// MODIFIED: Added worldOffsetX and worldOffsetY parameters
void TileMap::Render(Camera* camera, int worldOffsetX, int worldOffsetY) { 
    if (!tileset || map.empty() || !camera || tileWidth == 0 || tileHeight == 0) return;
// Camera's X and Y are absolute world coordinates
    float camX = camera->GetX(); 
    float camY = camera->GetY();
// Get camera's view dimensions
    int camViewWidth = camera->GetViewWidth();
    int camViewHeight = camera->GetViewHeight();

    // Determine the range of tiles to render based on camera and this chunk's world offset
    // The camera's X and Y are absolute world coordinates.
    // We need to find which part of *this specific chunk* is visible.
    // A tile at (tile_col, tile_row) within this chunk has a world position of:
    // (worldOffsetX + tile_col * tileWidth, worldOffsetY + tile_row * tileHeight)

    // Effective camera position relative to this chunk's origin
    // Calculate the effective camera position by subtracting the world offset
    // from the camera's position. This gives us the camera's position relative to this chunk coordinate system.
    
    float effectiveCamX = camX - worldOffsetX;
    float effectiveCamY = camY - worldOffsetY;

    // Calculate the start and end tile indices based on the camera's position
    int startCol = static_cast<int>(effectiveCamX / tileWidth);
    int endCol = static_cast<int>((effectiveCamX + camViewWidth) / tileWidth) + 1;
    int startRow = static_cast<int>(effectiveCamY / tileHeight);
    int endRow = static_cast<int>((effectiveCamY + camViewHeight) / tileHeight) + 1;

    // Clamp to this chunk's boundaries (0 to mapWidth/mapHeight in tiles)
    //Prevent out of tilemap's bounds rendering
    int clampedStartCol = std::max(0, startCol);
    int clampedEndCol = std::min(mapWidth, endCol);
    int clampedStartRow = std::max(0, startRow);
    int clampedEndRow = std::min(mapHeight, endRow);

    for (int row = clampedStartRow; row < clampedEndRow; ++row) {
        for (int column = clampedStartCol; column < clampedEndCol; ++column) {
            int tileId = map[row][column];
            if (tileId < 0) continue; // Skip empty tiles (-1 or other invalid)

            SDL_Rect srcRect;
            srcRect.x = (tileId % tilesetCols) * tileWidth;
            srcRect.y = (tileId / tilesetCols) * tileHeight;
            srcRect.w = tileWidth;
            srcRect.h = tileHeight;

            SDL_Rect dstRect;
            // Calculate the screen position of the tile relative to the camera
            // Tile's world X = worldOffsetX + columnolumn * tileWidth
            // Tile's screen X = (worldOffsetX + c * tileWidth) - camX
            dstRect.x = static_cast<int>(std::round((worldOffsetX + column * tileWidth) - camX));
            dstRect.y = static_cast<int>(std::round((worldOffsetY + row * tileHeight) - camY));
            dstRect.w = tileWidth;
            dstRect.h = tileHeight;

            SDL_RenderCopy(renderer, tileset, &srcRect, &dstRect);
        }
    }
}