# Snake - Multiplayer

## Overview
Snake game designed for multiple players. Multiple players aren't required to start a game. Uses Client - Server architecture. Clients connect to a server, server continously runs the game.

## Requirements
* Linux
* SDL2
* SDL_TTF

Game was created using Linux WSL, wasn't tested on LAN, only on localhost.

## Rules
* Each player gets one snake on connect
* PvP is enabled
* Purpose is to get the highest score
* Snakes are respawned on reconnect
* Game runs endlessly as long as server is running
