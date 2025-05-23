# Define paths to the unified SDL directories
SDL_DIR = ./lib

# Explicitly list all source directory patterns to ensure we capture everything
SOURCES = main.cpp \
	src/core/Game.cpp \
	src/entities/base/Tank.cpp \
	src/entities/base/Bullet.cpp \
	src/entities/powerup/PowerUp.cpp \
	src/utils/Explosion.cpp \
	src/utils/ParticleSystem.cpp \
	src/managers/ResourceManager.cpp \
	src/utils/KillNotification.cpp

# Default target - builds the game with all source files
all:
	g++ -I ./src \
		-I $(SDL_DIR)/SDL2/include \
		-I $(SDL_DIR)/SDL2/include/SDL2 \
		-I $(SDL_DIR)/SDL2_image/include/SDL2 \
		-I $(SDL_DIR)/SDL2_mixer/include/SDL2 \
		-I $(SDL_DIR)/SDL2_ttf/include/SDL2 \
		-L $(SDL_DIR)/SDL2/lib \
		-L $(SDL_DIR)/SDL2_image/lib \
		-L $(SDL_DIR)/SDL2_mixer/lib \
		-L $(SDL_DIR)/SDL2_ttf/lib \
		$(SOURCES) \
		-o main -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	@echo "Build complete."

