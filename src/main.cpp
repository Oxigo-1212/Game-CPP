#include "include/Game.h"

int main(int argc, char* argv[]) {
    Game game;
    
    if (!game.Initialize()) {
        return 1;
    }

    game.Run();
    return 0;
}