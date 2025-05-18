#include "include/TileMap.h"
#include "include/Camera.h" // Include Camera for its definition
#include <fstream>
#include <sstream>
#include <iostream>

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

void TileMap::Render(Camera* camera) { // Modified to take Camera*
    if (!tileset || map.empty() || !camera) return;

    // Get camera's view boundaries in world coordinates
    float camX = camera->GetX();
    float camY = camera->GetY();
    int camViewWidth = camera->GetViewWidth();
    int camViewHeight = camera->GetViewHeight();

    // Determine the range of tiles to render based on camera view (tile culling)
    int startCol = static_cast<int>(camX / tileWidth);
    int endCol = static_cast<int>((camX + camViewWidth) / tileWidth) + 1;
    int startRow = static_cast<int>(camY / tileHeight);
    int endRow = static_cast<int>((camY + camViewHeight) / tileHeight) + 1;

    // Clamp to map boundaries
    startCol = std::max(0, startCol);
    endCol = std::min(mapWidth, endCol);
    startRow = std::max(0, startRow);
    endRow = std::min(mapHeight, endRow);

    for (int y = startRow; y < endRow; ++y) {
        for (int x = startCol; x < endCol; ++x) {
            int tileId = map[y][x];
            if (tileId < 0) continue; // Skip empty tiles (-1 or other negative values)

            SDL_Rect srcRect;
            srcRect.x = (tileId % tilesetCols) * tileWidth;
            srcRect.y = (tileId / tilesetCols) * tileHeight;
            srcRect.w = tileWidth;
            srcRect.h = tileHeight;

            SDL_Rect dstRect;
            // Calculate world position of the tile
            float tileWorldX = static_cast<float>(x * tileWidth);
            float tileWorldY = static_cast<float>(y * tileHeight);

            // Convert to screen position
            dstRect.x = static_cast<int>(tileWorldX - camX);
            dstRect.y = static_cast<int>(tileWorldY - camY);
            dstRect.w = tileWidth;
            dstRect.h = tileHeight;

            SDL_RenderCopy(renderer, tileset, &srcRect, &dstRect);
        }
    }
}