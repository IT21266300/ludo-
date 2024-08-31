#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declare functions from game_logic.c
extern void initializeGame(GameState *game);
extern int rollDice();
extern void movePiece(GameState *game, int playerIndex, int pieceIndex, int steps);
extern void printGameStatus(GameState *game);
extern const char* getColorName(PlayerColor color);

int main() {

    

    srand(time(NULL));  // Initialize random seed
    
    GameState game;
    initializeGame(&game);
    
    printf("LUDO-CS Game Simulation\n\n");
    
    // Determine first player
    int highestRoll = 0;
    int firstPlayer = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        int roll = rollDice();
        printf("%s rolls %d\n", getColorName(game.players[i].color), roll);
        if (roll > highestRoll) {
            highestRoll = roll;
            firstPlayer = i;
        }
    }
    
    printf("%s player has the highest roll and will begin the game.\n", 
           getColorName(game.players[firstPlayer].color));
    
    game.currentPlayerIndex = firstPlayer;
    
    // Main game loop
    while (1) {
        Player *currentPlayer = &game.players[game.currentPlayerIndex];
        int roll = rollDice();
        
        printf("\n%s player rolled %d.\n", getColorName(currentPlayer->color), roll);

        while (roll == 6) {
        bool pieceMoved = false;

        // Try to move a piece out of the base
        for (int i = 0; i < PIECES_PER_PLAYER; i++) {
            if (currentPlayer->pieces[i].isBase) {
                movePiece(&game, game.currentPlayerIndex, i, roll);
                pieceMoved = true;
                break;
            }
        }

        // If no piece was moved out of the base, move a piece already on the board
        if (!pieceMoved) {
            for (int i = 0; i < PIECES_PER_PLAYER; i++) {
                if (!currentPlayer->pieces[i].isBase && !currentPlayer->pieces[i].isHome) {
                    movePiece(&game, game.currentPlayerIndex, i, roll);
                    break;
                }
            }
        }

        roll = rollDice();  // Roll again if a 6 was rolled
        printf("%s player rolled %d.\n", getColorName(currentPlayer->color), roll);
    }
        
        // Implement player behavior and move pieces here
        bool moved = false;
    for (int i = 0; i < PIECES_PER_PLAYER; i++) {
        if (!currentPlayer->pieces[i].isHome) {
            movePiece(&game, game.currentPlayerIndex, i, roll);
            moved = true;
            break;
        }
    }

    
        // This is where you'll need to add the logic for each player's behavior
        
        // For now, let's just move the first available piece
         implementPlayerBehaviors(&game, roll);
        
        
        // Check for win condition
        if (currentPlayer->piecesInHome == PIECES_PER_PLAYER) {
            printf("%s player wins!!!\n", getColorName(currentPlayer->color));
            break;
        }
        
        printGameStatus(&game);
        
        // Move to next player
        game.currentPlayerIndex = (game.currentPlayerIndex + 1) % NUM_PLAYERS;
        // Check if the round is complete (i.e., all players have taken their turns)
        if (game.currentPlayerIndex == 0) {
            game.roundCount++;
            // Update mystery cell if needed
            if (game.roundCount % 4 == 0) {
                // Implement mystery cell logic here
                if (game.roundCount % 4 == 0) {
    game.mysteryCell.position = rand() % BOARD_SIZE; // Randomly place the mystery cell
    game.mysteryCell.roundsLeft = 3; // It will stay for 3 rounds
    printf("A mystery cell has appeared at position %d!\n", game.mysteryCell.position);
} else if (game.mysteryCell.roundsLeft > 0) {
    game.mysteryCell.roundsLeft--;
    if (game.mysteryCell.roundsLeft == 0) {
        game.mysteryCell.position = -1; // Remove mystery cell
        printf("The mystery cell has disappeared.\n");
    }
}
            }
        }
    }
    
    // Wait for user input before closing
    printf("\nPress Enter to exit...");
    getchar();

    
    return 0;
}
