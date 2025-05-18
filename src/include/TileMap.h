#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>

class Camera; // Forward declaration

class TileMap {
private:
    SDL_Renderer* renderer;
    SDL_Texture* tileset;
    std::vector<std::vector<int>> map;
    int tileWidth;
    int tileHeight;
    int mapWidth;
    int mapHeight;
    int tilesetCols;

public:
    TileMap(SDL_Renderer* renderer);
    ~TileMap();

    bool LoadTileset(const char* path);
    bool LoadMap(const char* path);
    void Render(Camera* camera);

private:
    bool ParseCSV(const char* path);
};