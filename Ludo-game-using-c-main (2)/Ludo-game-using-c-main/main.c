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

     // Redirect stdout to a file
    FILE *outputFile = freopen("game_output.txt", "w", stdout);
    if (outputFile == NULL) {
        perror("Failed to open file for output");
        return 1;
    }
    
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
        
        // Implement player behavior and move pieces here
        // This is where you'll need to add the logic for each player's behavior
        
        // For now, let's just move the first available piece
        for (int i = 0; i < PIECES_PER_PLAYER; i++) {
            if (!currentPlayer->pieces[i].isHome) {
                movePiece(&game, game.currentPlayerIndex, i, roll);
                break;
            }
        }
        
        // Check for win condition
        if (currentPlayer->piecesInHome == PIECES_PER_PLAYER) {
            printf("%s player wins!!!\n", getColorName(currentPlayer->color));
            break;
        }
        
        printGameStatus(&game);
        
        // Move to next player
        game.currentPlayerIndex = (game.currentPlayerIndex + 1) % NUM_PLAYERS;
        game.roundCount++;
        
        // Update mystery cell if needed
        if (game.roundCount % 4 == 0) {
            // Implement mystery cell logic here
        }
    }
    
    // Wait for user input before closing
    printf("\nPress Enter to exit...");
    getchar();

    fclose(outputFile);
    
    return 0;
    
}