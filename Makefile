all: game

game: main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o
	g++ -Isrc/include -o game main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o -Lsrc/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

game.o: src/game.cpp src/include/Game.h src/include/Player.h src/include/UI.h
	g++ -Isrc/include -c src/game.cpp -o game.o

player.o: src/player.cpp src/include/Player.h
	g++ -Isrc/include -c src/player.cpp -o player.o

bullet.o: src/bullet.cpp src/include/Bullet.h
	g++ -Isrc/include -c src/bullet.cpp -o bullet.o

ui.o: src/UI.cpp src/include/UI.h
	g++ -Isrc/include -c src/UI.cpp -o ui.o

tilemap.o: src/TileMap.cpp src/include/TileMap.h
	g++ -Isrc/include -c src/TileMap.cpp -o tilemap.o

camera.o: src/Camera.cpp src/include/Camera.h
	g++ -Isrc/include -c src/Camera.cpp -o camera.o

ChunkManager.o: src/ChunkManager.cpp src/include/ChunkManager.h src/include/TileMap.h src/include/Player.h src/include/Camera.h
	g++ -Isrc/include -c src/ChunkManager.cpp -o ChunkManager.o

zombie.o: src/zombie.cpp src/include/Zombie.h src/include/Player.h src/include/Bullet.h
	g++ -Isrc/include -c src/zombie.cpp -o zombie.o

main.o: src/main.cpp src/include/Game.h
	g++ -Isrc/include -c src/main.cpp -o main.o

clean:
	-del /F /Q game.exe main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o 2>nul || rm -f game main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o

run:
	./game
