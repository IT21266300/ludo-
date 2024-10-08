#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define NUM_PLAYERS 4
#define PIECES_PER_PLAYER 4
#define BOARD_SIZE 52
#define HOME_PATH_SIZE 5

typedef enum
{
    YELLOW,
    BLUE,
    RED,
    GREEN
} PlayerColor;

typedef enum
{
    CLOCKWISE,
    COUNTERCLOCKWISE
} Direction;

typedef struct
{
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
    bool canMoveAgain;
} Piece;

typedef struct
{
    PlayerColor color;
    Piece pieces[PIECES_PER_PLAYER];
    int piecesInBase;
    int piecesInHome;
} Player;

typedef struct
{
    int position;
    int roundsLeft;
} MysteryCell;

typedef struct
{
    Player players[NUM_PLAYERS];
    MysteryCell mysteryCell;
    int currentPlayerIndex;
    int roundCount;
    int consecutiveSixesCount[NUM_PLAYERS];
} GameState;
void implementPlayerBehaviors(GameState *game, int diceRoll, int playerIndex);
// Function prototype for breakBlockade:
void breakBlockade(GameState *game, int playerIndex);
int distanceBetweenPieces(int pos1, int pos2);
bool canMoveBlock(GameState *game, int playerIndex, int pieceIndex, int steps);

#endif // TYPES_H