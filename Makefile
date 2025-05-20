all: game

game: main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o zombiepool.o wavemanager.o loadingscreen.o
	g++ -Isrc/include -o game main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o zombiepool.o wavemanager.o loadingscreen.o -Lsrc/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

game.o: src/game.cpp src/include/Game.h src/include/Player.h src/include/UI.h src/include/LoadingScreen.h
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

zombiepool.o: src/zombiepool.cpp src/include/ZombiePool.h src/include/Zombie.h
	g++ -Isrc/include -c src/zombiepool.cpp -o zombiepool.o

wavemanager.o: src/wavemanager.cpp src/include/WaveManager.h
	g++ -Isrc/include -c src/wavemanager.cpp -o wavemanager.o

loadingscreen.o: src/loadingscreen.cpp src/include/LoadingScreen.h
	g++ -Isrc/include -c src/loadingscreen.cpp -o loadingscreen.o

clean:
	-del /F /Q game.exe main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o zombiepool.o wavemanager.o loadingscreen.o 2>nul || rm -f game main.o game.o player.o bullet.o ui.o tilemap.o camera.o ChunkManager.o zombie.o zombiepool.o wavemanager.o loadingscreen.o

run:
	./game
