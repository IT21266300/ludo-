# Ludo-game-using-c

## Overview

LUDO-CS is a simple C-based console simulation of the classic Ludo board game. The game supports up to four players, each controlling four pieces of their color. Players take turns rolling a dice to move their pieces around the board, with the goal of getting all their pieces safely into their home.

## Features

- **Dice Rolling**: Players roll a dice on each turn to determine their movement.
- **Captures**: If a player lands on an opponent's piece, that piece is sent back to its base.
- **Home Path**: Players must move their pieces to the home area by rolling the exact number required.
- **Mystery Cell**: Every few rounds, a random event can occur, affecting gameplay.

## Files

- **`main.c`**: Contains the main function to start the game and manage the game loop.
- **`game_logic.c`**: Implements the core game logic, including player moves, dice rolls, and game rules.
- **`types.h`**: Defines the necessary data structures, such as player information, board status, and other types used across the project.

## How to Run

1. **Compile the code** using a C compiler like GCC:
   ```bash
   gcc -o ludo_simulation main.c game_logic.c -std=c99
2. **Run the compiled program**
   ```bash
   ./ludo_simulation
