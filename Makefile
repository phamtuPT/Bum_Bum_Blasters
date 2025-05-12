# Define paths to the unified SDL directories
SDL_DIR = ./lib

# Explicitly list all source directory patterns to ensure we capture everything
SOURCES = main.cpp \
	$(wildcard src/*.cpp) \
	$(wildcard src/core/*.cpp) \
	$(wildcard src/entities/*.cpp) \
	$(wildcard src/entities/base/*.cpp) \
	$(wildcard src/entities/enemy/*.cpp) \
	$(wildcard src/entities/player/*.cpp) \
	$(wildcard src/entities/powerup/*.cpp) \
	$(wildcard src/managers/*.cpp) \
	$(wildcard src/states/*.cpp) \
	$(wildcard src/systems/*.cpp) \
	$(wildcard src/ui/*.cpp) \
	$(wildcard src/utils/*.cpp) 

# Default target - builds the game with all source files
all:
	g++ -I ./include \
		-I $(SDL_DIR)/SDL2/include \
		-I $(SDL_DIR)/SDL2/include/SDL2 \
		-I $(SDL_DIR)/SDL2_image/include \
		-I $(SDL_DIR)/SDL2_mixer/include \
		-I $(SDL_DIR)/SDL2_ttf/include \
		-L $(SDL_DIR)/SDL2/lib \
		-L $(SDL_DIR)/SDL2_image/lib \
		-L $(SDL_DIR)/SDL2_mixer/lib \
		-L $(SDL_DIR)/SDL2_ttf/lib \
		$(SOURCES) \
		-o main -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	@echo "Build complete."

