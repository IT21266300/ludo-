#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define NUM_PLAYERS 4
#define PIECES_PER_PLAYER 4
#define BOARD_SIZE 52
#define HOME_PATH_SIZE 5

typedef enum {
    RED,
    GREEN,
    YELLOW,
    BLUE
} PlayerColor;

typedef enum {
    CLOCKWISE,
    COUNTERCLOCKWISE
} Direction;

typedef struct {
    int id;
    PlayerColor color;
    int position;
    bool isHome;
    bool isBase;
    Direction direction;
    int captures;
    bool isEnergized;
    bool isSick;
    int briefingRoundsLeft;
} Piece;

typedef struct {
    PlayerColor color;
    Piece pieces[PIECES_PER_PLAYER];
    int piecesInBase;
    int piecesInHome;
} Player;

typedef struct {
    int position;
    int roundsLeft;
} MysteryCell;

typedef struct {
    Player players[NUM_PLAYERS];
    MysteryCell mysteryCell;
    int currentPlayerIndex;
    int roundCount;
} GameState;

#endif // TYPES_H