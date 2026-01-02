cmake_minimum_required(VERSION 3.25)
project(game C)

set(CMAKE_C_STANDARD 11)

# Shared sources for the game logic
set(GAME_SOURCES
        game/snake.c
        game/game.c
        game/fruit.c
)

# Singleplayer executable
add_executable(game
        ${GAME_SOURCES}
        gui/game_render.c
        main.c           # your singleplayer main
)

# Server executable (no SDL needed)
add_executable(server
        ${GAME_SOURCES}
        server/server.c
)

# Client executable (GUI)
add_executable(client
        ${GAME_SOURCES}
        gui/game_render.c
        client/client.c
)

# Find SDL2 and SDL2_ttf via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include directories
target_include_directories(game PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(game PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

target_include_directories(client PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(client PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

