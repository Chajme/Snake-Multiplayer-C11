cmake_minimum_required(VERSION 3.25)
project(snake_game C)

set(CMAKE_C_STANDARD 11)

# Game Sources
set(GAME_SOURCES
        util/queue.c
        util/vector.c
        server/game/snake.c
        server/game/game.c
        server/game/fruit.c
        server/game_controller.c
        server/server.c
)

# SDL LEAK TEST
add_executable(sdl_leak_test
        sdl_leak_test.c
)

# Server
add_executable(server
        ${GAME_SOURCES}
        server/server_main.c
)

# Client
add_executable(client
        ${GAME_SOURCES}
        client/gui/game_render.c
        client/client.c
        client/client_main.c
)

# Find SDL2 and SDL2_ttf via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Includes
target_include_directories(sdl_leak_test PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(sdl_leak_test PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

target_include_directories(client PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(client PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

