#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
int rollDice();
void initializeGame(GameState *game);
void movePiece(GameState *game, int playerIndex, int pieceIndex, int steps);
const char *getColorName(PlayerColor color);
void printGameStatus(GameState *game);
void checkForCaptures(GameState *game, int playerIndex, int pieceIndex);
void handleMysteryCell(GameState *game, int playerIndex, int pieceIndex);
bool checkForWin(GameState *game, int playerIndex);
void implementPlayerBehaviors(GameState *game, int diceRoll, int playerIndex);
void moveBlock(GameState *game, int playerIndex, int pieceIndex, int steps);
bool isBlockCreated(GameState *game, int position);
void breakBlockade(GameState *game, int playerIndex);
void teleportPiece(GameState *game, int playerIndex, int pieceIndex, int destination);

int rollDice()
{
    return (rand() % 6) + 1;
}
int distanceBetweenPieces(int pos1, int pos2)
{
    int dist = abs(pos1 - pos2);
    int circularDist = BOARD_SIZE - dist;
    return (dist < circularDist) ? dist : circularDist;
}

bool canMoveBlock(GameState *game, int playerIndex, int pieceIndex, int steps) {
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    // 1. Cannot move if the piece is in the base or home
    if (piece->isBase || piece->isHome) {
        return false;
    }

    // 2. Calculate the potential new position
    int newPosition = (piece->position + steps) % BOARD_SIZE;

    // 3. Check if a block is ALREADY created at the new position
    if (isBlockCreated(game, newPosition)) {
        return false; // Cannot move into an existing block
    }

    // 4. Check if moving to the new position WOULD create a block
    //    (This assumes a block requires at least two pieces)
    int piecesAtNewPosition = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < PIECES_PER_PLAYER; j++) {
            Piece *p = &game->players[i].pieces[j];
            // Count pieces at the new position (excluding the current piece being moved)
            if (p->position == newPosition && !p->isBase && !p->isHome && !(i == playerIndex && j == pieceIndex)) { 
                piecesAtNewPosition++;
            }
        }
    }

    // A block would be created if there's at least one other piece at the new position
    return (piecesAtNewPosition > 0); 
}

void initializeGame(GameState *game)
{
    int startingPositions[NUM_PLAYERS] = {2, 15, 28, 41}; // Yellow, Blue, Red, Green
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        game->players[i].color = i;
        game->players[i].piecesInBase = PIECES_PER_PLAYER;
        game->players[i].piecesInHome = 0;
        for (int j = 0; j < PIECES_PER_PLAYER; j++)
        {
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
    game->roundCount = 1;
}

void movePiece(GameState *game, int playerIndex, int pieceIndex, int steps)
{
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];
    int startingPosition = (playerIndex * 13 + 2) % BOARD_SIZE; // Adjusting for different start positions

    if (piece->isBase && steps == 6)
    {
        piece->isBase = false;
        piece->position = startingPosition;
        game->players[playerIndex].piecesInBase--;
        printf("%s player moves piece %d to the starting point.\n",
               getColorName(piece->color), piece->id);
    }
    else if (!piece->isBase && !piece->isHome)
    {
        int newPosition;
        if (piece->direction == CLOCKWISE)
        {
            newPosition = (piece->position + steps) % BOARD_SIZE;
        }
        else
        {
            newPosition = (piece->position - steps + BOARD_SIZE) % BOARD_SIZE;
        }

       // Check if the piece is at its CORRECT home entrance AND has enough moves
       if (newPosition == startingPosition && piece->captures > 0) 
        {
            // Calculate the position WITHIN the home path (0-4)
            int homePathPosition = steps - 1; 

            // Check if the piece would overshoot the home
            if (homePathPosition <= HOME_PATH_SIZE)
            {
                piece->position = playerIndex * HOME_PATH_SIZE + homePathPosition; // Calculate absolute home position
                printf("%s moves piece %d to home path position %d.\n",
                       getColorName(piece->color), piece->id, homePathPosition + 1); // Display 1-based position

                if (homePathPosition == HOME_PATH_SIZE) 
                {
                    piece->isHome = true;
                    game->players[playerIndex].piecesInHome++;
                    printf("%s piece %d has reached home!\n", getColorName(piece->color), piece->id);
                }
            }
            else
            {
                printf("%s piece %d cannot move as it would overshoot home.\n", getColorName(piece->color), piece->id);
                return; // Don't move the piece
            }
        }
        else
        {
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

void checkForCaptures(GameState *game, int playerIndex, int pieceIndex)
{
    Piece *movingPiece = &game->players[playerIndex].pieces[pieceIndex];

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        if (i != playerIndex)
        {
            for (int j = 0; j < PIECES_PER_PLAYER; j++)
            {
                Piece *otherPiece = &game->players[i].pieces[j];
                if (!otherPiece->isBase && !otherPiece->isHome &&
                    otherPiece->position == movingPiece->position)
                {

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

void handleMysteryCell(GameState *game, int playerIndex, int pieceIndex)
{
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    if (game->mysteryCell.position != -1 && piece->position == game->mysteryCell.position)
    {
        printf("%s player's piece %d landed on the mystery cell!\n",
               getColorName(piece->color), piece->id);

        // Randomly select teleport destination
        int destination = rand() % 6;
        const char *destinations[] = {"Bhawana", "Kotuwa", "Pita-Kotuwa", "Base", "X", "Approach"};
        printf("%s piece %d teleported to %s.\n", getColorName(piece->color), piece->id, destinations[destination]);

        teleportPiece(game, playerIndex, pieceIndex, destination);

        game->mysteryCell.position = -1; // Reset mystery cell after use
    }
}

void teleportPiece(GameState *game, int playerIndex, int pieceIndex, int destination)
{
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    switch (destination)
    {
    case 0: // Bhawana
        piece->position = 9;
        if (rand() % 2 == 0)
        {
            piece->isEnergized = true;
            printf("%s piece %d feels energized, and movement speed doubles.\n", getColorName(piece->color), piece->id);
        }
        else
        {
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
        if (piece->direction == CLOCKWISE)
        {
            piece->direction = COUNTERCLOCKWISE;
            printf("The %s piece %d, which was moving clockwise, has changed to moving counterclockwise.\n", getColorName(piece->color), piece->id);
        }
        else
        {
            printf("The %s piece %d is moving in a counterclockwise direction. Teleporting to Kotuwa from Pita-Kotuwa.\n", getColorName(piece->color), piece->id);
            teleportPiece(game, playerIndex, pieceIndex, 1); // Teleport to Kotuwa
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

bool checkForWin(GameState *game, int playerIndex)
{
    return game->players[playerIndex].piecesInHome == PIECES_PER_PLAYER;
}

// Function to find a random movable piece for a given player
int findRandomMovablePiece(Player* player) {
    int movablePieces[PIECES_PER_PLAYER];
    int numMovablePieces = 0;
    for (int i = 0; i < PIECES_PER_PLAYER; i++) {
        if (!player->pieces[i].isBase && !player->pieces[i].isHome) {
            movablePieces[numMovablePieces++] = i;
        }
    }
    if (numMovablePieces > 0) {
        return movablePieces[rand() % numMovablePieces];
    }
    return -1; // No movable pieces found
}

void implementPlayerBehaviors(GameState *game, int diceRoll, int playerIndex) {
    Player *currentPlayer = &game->players[playerIndex];
    int startingPosition = (playerIndex * 13 + 2) % BOARD_SIZE;
    PlayerColor color = currentPlayer->color;

    // Helper function to find a random movable piece
    // (You might want to move this outside implementPlayerBehaviors 
    // to avoid code repetition)
    

    switch (color) {
        case RED: {
            // 1. Prioritize Capturing:
            int nearestOpponentDistance = BOARD_SIZE + 1;
            int pieceToMove = -1;

            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i]; 
                if (!piece->isBase && !piece->isHome) {
                    for (int j = 0; j < NUM_PLAYERS; j++) {
                        if (j != game->currentPlayerIndex) {
                            for (int k = 0; k < PIECES_PER_PLAYER; k++) {
                                Piece *opponentPiece = &game->players[j].pieces[k];
                                if (!opponentPiece->isBase && !opponentPiece->isHome) {
                                    int distanceToOpponent = distanceBetweenPieces(piece->position, opponentPiece->position);
                                    if (distanceToOpponent == diceRoll && distanceToOpponent < nearestOpponentDistance) {
                                        nearestOpponentDistance = distanceToOpponent;
                                        pieceToMove = i; 
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (pieceToMove != -1) {
                movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                printf("RED player is moving to capture!\n");
                return; 
            }

            // 2. Move Piece from Base:
            if (diceRoll == 6 && currentPlayer->piecesInBase > 0) {
                pieceToMove = -1;
                for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                    if (currentPlayer->pieces[i].isBase) {
                        pieceToMove = i;
                        break;
                    }
                }

                if (pieceToMove != -1) { 
                    movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                    printf("RED player moves a piece from base to X.\n");
                    return;
                } 
            }

            // 3. Move other movable pieces:
            pieceToMove = findRandomMovablePiece(currentPlayer);
            if (pieceToMove != -1) {
                movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                printf("RED player moves a piece already on the board.\n");
                return;
            }
            break;
        }

        case GREEN: {
    // 1. Move from Base to X if possible (considering blocks):
    if (diceRoll == 6 && currentPlayer->piecesInBase > 0) {
        int pieceToMove = -1;
        for (int i = 0; i < PIECES_PER_PLAYER; i++) {
            if (currentPlayer->pieces[i].isBase) {
                // Check if moving to X would create a block, but allow if less than 2 pieces in base
                if (!(isBlockCreated(game, startingPosition) && currentPlayer->piecesInBase <= 2)) { 
                    pieceToMove = i;
                    break;
                } else {
                    printf("GREEN player chooses to keep a piece in the base to avoid a potential block.\n");
                }
            }
        }

        if (pieceToMove != -1) {
            movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
            printf("GREEN player moves a piece from the base to X.\n");
            return;  
        }
    }

    // 2. Attempt to create or maintain a block:
    int pieceToMove = -1;
    for (int i = 0; i < PIECES_PER_PLAYER; i++) {
        if (canMoveBlock(game, game->currentPlayerIndex, i, diceRoll)) { // Check if a block can be made
            pieceToMove = i;
            break;
        }
    }
    if (pieceToMove != -1) {
        moveBlock(game, game->currentPlayerIndex, pieceToMove, diceRoll); // Move only if a block can be made
        return; // Only move one piece per roll
    }

    // 3. Move other pieces towards home (randomly):
    pieceToMove = findRandomMovablePiece(currentPlayer);
    if (pieceToMove != -1) {
        movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
        printf("GREEN player moves a piece already on the board.\n");
        return; 
    }
    break; 
}

        case YELLOW: { 
            // 1. Move from base to X if possible
            if (diceRoll == 6 && currentPlayer->piecesInBase > 0) {
                int pieceToMove = -1;
                for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                    if (currentPlayer->pieces[i].isBase) {
                        pieceToMove = i;
                        break;
                    }
                }
                if (pieceToMove != -1) {
                    movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                    printf("YELLOW player moves a piece from the base to X.\n");
                    return; 
                }
            }

            // 2. Capture if possible 
            int pieceToMove = -1;
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isBase && !piece->isHome && piece->captures < 1) { 
                    for (int j = 0; j < NUM_PLAYERS; j++) {
                        if (j != game->currentPlayerIndex) {
                            for (int k = 0; k < PIECES_PER_PLAYER; k++) {
                                Piece *opponentPiece = &game->players[j].pieces[k];
                                if (!opponentPiece->isBase && !opponentPiece->isHome) {
                                    int distanceToOpponent = distanceBetweenPieces(piece->position, opponentPiece->position);
                                    if (distanceToOpponent == diceRoll) {
                                        pieceToMove = i;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (pieceToMove != -1) {
                movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                printf("YELLOW player is moving to capture to enter the home path!\n");
                return; 
            }

            // 3. Prioritize moving the piece closest to home
            int closestToHomeDistance = BOARD_SIZE + 1;
            pieceToMove = -1;

            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                Piece *piece = &currentPlayer->pieces[i];
                if (!piece->isHome && !piece->isBase) {
                    int distanceFromHome = distanceBetweenPieces(piece->position, startingPosition);
                    if (distanceFromHome < closestToHomeDistance) {
                        closestToHomeDistance = distanceFromHome;
                        pieceToMove = i;
                    }
                }
            }

            if (pieceToMove != -1) {
                movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
                // ... (rest of your YELLOW player logic) ...
                return; 
            }
            break; 
        }

        case BLUE: {
    // 1. Move the piece that has been on the board the longest 
    int pieceToMove = -1;
    int oldestRound = game->roundCount; // Initialize with a high round number

    for (int i = 0; i < PIECES_PER_PLAYER; i++) {
        Piece *piece = &currentPlayer->pieces[i];
        if (!piece->isBase && !piece->isHome) {
            // Track when each piece left the base 
            if (piece->position >= 0 && piece->position < BOARD_SIZE) { // Check if piece is on main track
                int roundsOnBoard = game->roundCount - (piece->briefingRoundsLeft + 1); // Approximation
                if (roundsOnBoard < oldestRound) {
                    oldestRound = roundsOnBoard;
                    pieceToMove = i;
                }
            }
        }
    }

    if (pieceToMove != -1) {
        movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
        printf("BLUE player moves the oldest piece on the board.\n");
        return; 
    }

    // 2. If no pieces are on the board, try to move a piece from the base
    if (diceRoll == 6 && currentPlayer->piecesInBase > 0) { 
        for (int i = 0; i < PIECES_PER_PLAYER; i++) {
            if (currentPlayer->pieces[i].isBase) {
                pieceToMove = i;
                break;
            }
        }
        if (pieceToMove != -1) {
            movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
            printf("BLUE player moves a piece from the base to X.\n");
            return; 
        }
    }

    // 3. Move other movable pieces (randomly) if no older piece can move
    pieceToMove = findRandomMovablePiece(currentPlayer);
    if (pieceToMove != -1) {
        movePiece(game, game->currentPlayerIndex, pieceToMove, diceRoll);
        printf("BLUE player moves a piece randomly.\n");
        return; 
    }
    break; 
}
    }
}

// Move block logic (CS-4)
void moveBlock(GameState *game, int playerIndex, int pieceIndex, int steps)
{
    Piece *piece = &game->players[playerIndex].pieces[pieceIndex];

    if (piece->isBase || piece->isHome)
    {
        return; // Cannot move if in base or home
    }

    int newPosition = (piece->position + steps) % BOARD_SIZE;

    // Check if a block is created at the new position
    if (isBlockCreated(game, newPosition))
    {
        printf("A block is already created at the new position.\n");
        return;
    }

    piece->position = newPosition;
    printf("Block moved to position %d.\n", newPosition);
}

// Check if a block is created at a given position (CS-5)
bool isBlockCreated(GameState *game, int position)
{
    int pieceCount = 0;

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        for (int j = 0; j < PIECES_PER_PLAYER; j++)
        {
            Piece *piece = &game->players[i].pieces[j];
            if (piece->position == position && !piece->isHome && !piece->isBase)
            {
                pieceCount++;
            }
        }
    }

    return pieceCount >= 2;
}

// Break blockade logic (CS-6)
void breakBlockade(GameState *game, int playerIndex)
{
    Player *player = &game->players[playerIndex];

    for (int i = 0; i < PIECES_PER_PLAYER; i++)
    {
        Piece *piece = &player->pieces[i];
        if (!piece->isHome && !piece->isBase)
        {
            int blockPosition = piece->position;
            if (isBlockCreated(game, blockPosition))
            {
                // Logic to break the blockade
                // Placeholder for breaking the block
                printf("Blockade at position %d broken by player %s.\n", blockPosition, getColorName(player->color));
                break;
            }
        }
    }
}

const char *getColorName(PlayerColor color)
{
    switch (color)
    {
    case RED:
        return "Red";
    case GREEN:
        return "Green";
    case YELLOW:
        return "Yellow";
    case BLUE:
        return "Blue";
    default:
        return "Unknown";
    }
}

void printGameStatus(GameState *game)
{

    printf("Round: %d\n", game->roundCount);

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        printf("%s player now has %d/4 pieces on the board and %d/4 pieces on the base.\n",
               getColorName(game->players[i].color),
               PIECES_PER_PLAYER - game->players[i].piecesInBase - game->players[i].piecesInHome,
               game->players[i].piecesInBase);

        printf("============================\n");
        printf("Location of pieces %s\n", getColorName(game->players[i].color));
        printf("============================\n");

        for (int j = 0; j < PIECES_PER_PLAYER; j++)
        {
            Piece *piece = &game->players[i].pieces[j];
            if (piece->isBase)
            {
                printf("Piece %d -> Base\n", piece->id);
            }
            else if (piece->isHome)
            {
                printf("Piece %d -> Home\n", piece->id);
            }
            else
            {
                printf("Piece %d -> %d\n", piece->id, piece->position);
            }
        }
        printf("\n");
    }

    if (game->mysteryCell.position != -1)
    {
        printf("The mystery cell is at %d and will be at that location for the next %d rounds.\n",
               game->mysteryCell.position, game->mysteryCell.roundsLeft);
    }
}