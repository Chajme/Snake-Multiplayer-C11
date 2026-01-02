cmake_minimum_required(VERSION 3.25)
project(SnakeGame C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)

# ---- Server ----
add_executable(server
        server/server.c
        server/client_handler.c
        server/server_network.c
        game/game.c
)
target_include_directories(server PRIVATE common game server)
target_link_libraries(server pthread)

# ---- Client ----
add_executable(client
        client/client.c
        client/client_network.c
        client/game_render.c
        game/game.c
)
target_include_directories(client PRIVATE common game client)
target_link_libraries(client ${SDL2_LIBRARIES} pthread)

# ---- SDL test ----
add_executable(test main.c)
target_include_directories(test PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(test ${SDL2_LIBRARIES})
