cmake_minimum_required(VERSION 3.25)
project(udsp_semestralka_3 C)

set(CMAKE_C_STANDARD 11)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} common)

# Server
add_executable(server
        server/server.c
        server/client_handler.c
        game/game.c
)
target_include_directories(server PRIVATE server common)
target_link_libraries(server pthread)

# Client
add_executable(client
        client/client.c
        client/game_render.c
)
target_include_directories(client PRIVATE client common)
target_link_libraries(client ${SDL2_LIBRARIES} pthread)
