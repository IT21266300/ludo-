#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
int rollDice();
void initializeGame(GameState *game);
void movePiece(GameState *game, int playerIndex, int pieceIndex, int steps);
const char* getColorName(PlayerColor color);
void printGameStatus(GameState *game);
void checkForCaptures(GameState *game, int playerIndex, int pieceIndex);
void handleMysteryCell(GameState *game, int playerIndex, int pieceIndex);
bool checkForWin(GameState *game, int playerIndex);
void implementPlayerBehaviors(GameState *game, int diceRoll);
void moveBlock(GameState *game, int playerIndex, int pieceIndex, int steps);
bool isBlockCreated(GameState *game, int position);
void breakBlockade(GameState *game, int playerIndex);
void teleportPiece(GameState *game, int playerIndex, int pieceIndex, int destination);

int rollDice() {
    return (rand() % 6) + 1;
}

void initializeGame(GameState *game) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        game->players[i].color = i;
        game->players[i].piecesInBase = PIECES_PER_PLAYER;
        game->players[i].piecesInHome = 0;
        for (int j = 0; j < PIECES_PER_PLAYER; j++) {
            game->players[i].pieces[j].id = j + 1;
            game->players[i].pieces[j].color = i;
            game->players[i].pieces[j].isBase = true;
            game->players[i].pieces[j].isHome = false;
            game->players[i].pieces[j].position = -1;
            game->players[i].pieces[j].direction = CLOCKWISE;
            game->players[i].pieces[j].captures = 0;
            game->players[i].pieces[j].isEnergized = false;
            game->players[i].pieces[j].isSick = false;
            game->players[i].pieces[j].briefingRoundsLeft = 0;
        }
    }
    game->mysteryCell.position = -1;
    game->mysteryCell.roundsLeft = 0;
    game->currentPlayerIndex = 0;
    game->roundCount = 0;
}

void movePiece(GameState *game, int playerIndex, int pieceIndex, int steps) {
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    if (piece->isBase && steps == 6) {
        piece->isBase = false;
        piece->position = playerIndex * (BOARD_SIZE / NUM_PLAYERS);
        game->players[playerIndex].piecesInBase--;
        printf("%s player moves piece %d to the starting point.\n", 
               getColorName(piece->color), piece->id);
    } else if (!piece->isBase && !piece->isHome) {
        int newPosition;
        if (piece->direction == CLOCKWISE) {
            newPosition = (piece->position + steps) % BOARD_SIZE;
        } else {
            newPosition = (piece->position - steps + BOARD_SIZE) % BOARD_SIZE;
        }

        // Check if the piece is entering home straight
        if (piece->captures > 0 && 
            ((piece->direction == CLOCKWISE && newPosition >= playerIndex * (BOARD_SIZE / NUM_PLAYERS) + BOARD_SIZE / NUM_PLAYERS) ||
             (piece->direction == COUNTERCLOCKWISE && newPosition <= playerIndex * (BOARD_SIZE / NUM_PLAYERS)))) {
            // Enter home straight
            int homePathPosition = steps - (BOARD_SIZE / NUM_PLAYERS - (piece->position % (BOARD_SIZE / NUM_PLAYERS)));
            if (homePathPosition <= HOME_PATH_SIZE) {
                piece->position = playerIndex * HOME_PATH_SIZE + homePathPosition - 1;
                printf("%s moves piece %d to home path position %d.\n",
                       getColorName(piece->color), piece->id, homePathPosition);
                if (homePathPosition == HOME_PATH_SIZE) {
                    piece->isHome = true;
                    game->players[playerIndex].piecesInHome++;
                    printf("%s piece %d has reached home!\n", getColorName(piece->color), piece->id);
                }
            } else {
                printf("%s piece %d cannot move as it would overshoot home.\n", getColorName(piece->color), piece->id);
                return;
            }
        } else {
            // Normal movement
            printf("%s moves piece %d from location %d to %d by %d units in %s direction.\n",
                   getColorName(piece->color), piece->id, piece->position, newPosition, 
                   steps, (piece->direction == CLOCKWISE) ? "clockwise" : "counterclockwise");
            piece->position = newPosition;
        }

        // Check for captures, mystery cells, etc.
        checkForCaptures(game, playerIndex, pieceIndex);
        handleMysteryCell(game, playerIndex, pieceIndex);
    }
}

void checkForCaptures(GameState *game, int playerIndex, int pieceIndex) {
    Piece *movingPiece = &game->players[playerIndex].pieces[pieceIndex];

    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (i != playerIndex) {
            for (int j = 0; j < PIECES_PER_PLAYER; j++) {
                Piece *otherPiece = &game->players[i].pieces[j];
                if (!otherPiece->isBase && !otherPiece->isHome &&
                    otherPiece->position == movingPiece->position) {

                    otherPiece->isBase = true;
                    otherPiece->position = -1;
                    game->players[i].piecesInBase++;
                    movingPiece->captures++;

                    printf("%s player captures %s player's piece %d!\n",
                           getColorName(movingPiece->color), 
                           getColorName(otherPiece->color), otherPiece->id);
                    
                    // Rule CS-2: Bonus roll for capture
                    printf("%s player gets a bonus roll for capturing.\n", getColorName(movingPiece->color));
                    int bonusRoll = rollDice();
                    printf("%s player rolled %d for the bonus.\n", getColorName(movingPiece->color), bonusRoll);
                    movePiece(game, playerIndex, pieceIndex, bonusRoll);
                }
            }
        }
    }
}

void handleMysteryCell(GameState *game, int playerIndex, int pieceIndex) {
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    if (game->mysteryCell.position != -1 && piece->position == game->mysteryCell.position) {
        printf("%s player's piece %d landed on the mystery cell!\n", 
               getColorName(piece->color), piece->id);

        // Randomly select teleport destination
        int destination = rand() % 6;
        const char* destinations[] = {"Bhawana", "Kotuwa", "Pita-Kotuwa", "Base", "X", "Approach"};
        printf("%s piece %d teleported to %s.\n", getColorName(piece->color), piece->id, destinations[destination]);

        teleportPiece(game, playerIndex, pieceIndex, destination);

        game->mysteryCell.position = -1;  // Reset mystery cell after use
    }
}

void teleportPiece(GameState *game, int playerIndex, int pieceIndex, int destination) {
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    switch(destination) {
        case 0: // Bhawana
            piece->position = 9;
            if (rand() % 2 == 0) {
                piece->isEnergized = true;
                printf("%s piece %d feels energized, and movement speed doubles.\n", getColorName(piece->color), piece->id);
            } else {
                piece->isSick = true;
                printf("%s piece %d feels sick, and movement speed halves.\n", getColorName(piece->color), piece->id);
            }
            break;
        case 1: // Kotuwa
            piece->position = 2;
            piece->briefingRoundsLeft = 4;
            printf("%s piece %d attends briefing and cannot move for four rounds.\n", getColorName(piece->color), piece->id);
            break;
        case 2: // Pita-Kotuwa
            piece->position = 46;
            if (piece->direction == CLOCKWISE) {
                piece->direction = COUNTERCLOCKWISE;
                printf("The %s piece %d, which was moving clockwise, has changed to moving counterclockwise.\n", getColorName(piece->color), piece->id);
            } else {
                printf("The %s piece %d is moving in a counterclockwise direction. Teleporting to Kotuwa from Pita-Kotuwa.\n", getColorName(piece->color), piece->id);
                teleportPiece(game, playerIndex, pieceIndex, 1);  // Teleport to Kotuwa
            }
            break;
        case 3: // Base
            piece->isBase = true;
            piece->position = -1;
            game->players[playerIndex].piecesInBase++;
            break;
        case 4: // X
            piece->position = playerIndex * (BOARD_SIZE / NUM_PLAYERS);
            break;
        case 5: // Approach
            piece->position = ((playerIndex * (BOARD_SIZE / NUM_PLAYERS)) + (BOARD_SIZE / NUM_PLAYERS) - 1) % BOARD_SIZE;
            break;
    }
}

bool checkForWin(GameState *game, int playerIndex) {
    return game->players[playerIndex].piecesInHome == PIECES_PER_PLAYER;
}




void implementPlayerBehaviors(GameState *game, int diceRoll) {
    Player *currentPlayer = &game->players[game->currentPlayerIndex];
    PlayerColor color = currentPlayer->color;

    switch(color) {
        case RED:
            // Implement Red player's aggressive behavior
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isHome && !piece->isBase) {
                    // Move towards the nearest opponent piece
                    printf("RED player is moving aggressively.\n");
                    break;
                }
            }
            break;
        case GREEN:
            // Implement Green player's blocking strategy
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isHome && !piece->isBase) {
                    if (isBlockCreated(game, piece->position)) {
                        printf("GREEN player maintains the block.\n");
                    } else {
                        moveBlock(game, game->currentPlayerIndex, i, diceRoll);
                        printf("GREEN player is creating a block.\n");
                    }
                    break;
                }
            }
            break;
        case YELLOW:
            // Implement Yellow player's winning strategy
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isHome && !piece->isBase) {
                    // Move pieces closest to home
                    printf("YELLOW player is focusing on winning.\n");
                    break;
                }
            }
            break;
        case BLUE:
            // Implement Blue player's random behavior
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isHome && !piece->isBase) {
                    // Assuming MysteryCell has a 'position' field
                    if (piece->position == game->mysteryCell.position) {
                        printf("BLUE player landed on a mystery cell!\n");
                        // Implement mystery cell behavior
                    } else {
                        // Move counterclockwise
                        printf("BLUE player moves randomly.\n");
                    }
                    break;
                }
            }
            break;
    }
}





// Move block logic (CS-4)
void moveBlock(GameState *game, int playerIndex, int pieceIndex, int steps) {
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];
    
    if (piece->isBase || piece->isHome) {
        return;  // Cannot move if in base or home
    }

    int newPosition = (piece->position + steps) % BOARD_SIZE;
    
    // Check if a block is created at the new position
    if (isBlockCreated(game, newPosition)) {
        printf("A block is already created at the new position.\n");
        return;
    }

    piece->position = newPosition;
    printf("Block moved to position %d.\n", newPosition);
}

// Check if a block is created at a given position (CS-5)
bool isBlockCreated(GameState *game, int position) {
    int pieceCount = 0;

    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < PIECES_PER_PLAYER; j++) {
            Piece *piece = &game->players[i].pieces[j];
            if (piece->position == position && !piece->isHome && !piece->isBase) {
                pieceCount++;
            }
        }
    }

    return pieceCount >= 2;
}

// Break blockade logic (CS-6)
void breakBlockade(GameState *game, int playerIndex) {
    Player *player = &game->players[playerIndex];

    for (int i = 0; i < PIECES_PER_PLAYER; i++) {
        Piece *piece = &player->pieces[i];
        if (!piece->isHome && !piece->isBase) {
            int blockPosition = piece->position;
            if (isBlockCreated(game, blockPosition)) {
                // Logic to break the blockade
                // Placeholder for breaking the block
                printf("Blockade at position %d broken by player %s.\n", blockPosition, getColorName(player->color));
                break;
            }
        }
    }
}

const char* getColorName(PlayerColor color) {
    switch(color) {
        case RED: return "Red";
        case GREEN: return "Green";
        case YELLOW: return "Yellow";
        case BLUE: return "Blue";
        default: return "Unknown";
    }
}

void printGameStatus(GameState *game) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("%s player now has %d/4 pieces on the board and %d/4 pieces on the base.\n",
               getColorName(game->players[i].color),
               PIECES_PER_PLAYER - game->players[i].piecesInBase - game->players[i].piecesInHome,
               game->players[i].piecesInBase);

        printf("============================\n");
        printf("Location of pieces %s\n", getColorName(game->players[i].color));
        printf("============================\n");

        for (int j = 0; j < PIECES_PER_PLAYER; j++) {
            Piece *piece = &game->players[i].pieces[j];
            if (piece->isBase) {
                printf("Piece %d -> Base\n", piece->id);
            } else if (piece->isHome) {
                printf("Piece %d -> Home\n", piece->id);
            } else {
                printf("Piece %d -> %d\n", piece->id, piece->position);
            }
        }
        printf("\n");
    }

    if (game->mysteryCell.position != -1) {
        printf("The mystery cell is at %d and will be at that location for the next %d rounds.\n",
               game->mysteryCell.position, game->mysteryCell.roundsLeft);
    }
}