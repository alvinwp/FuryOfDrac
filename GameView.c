// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "string.h"

// #include "Map.h" ... if you decide to use the Map ADT

struct gameView {
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    ////////////////////////////////////////////
    //All information required to "view" the game here
    //Get all this from functions below
    //e.g get round will simply gameview->round,
    int round;
    int currentPlayer;
    int score;
    ////////////////////////////////////////////
    ///////Structs
    //Individual arrays for each of size 5;(0,1,2,3 = hunters)4=drac
    int *health;
    int **path;
    int *upto;
    //Not sure on how to store path history/past plays, either as 5 arrays as given
    //or a linked list, we will have to implement our own ll though(easier to keep
    //track of turns)
};

void addTrapVamp(LocationID ID, char *num, int **time) {
    time[ID][num[ID]] = 6+1;
    num[ID] += 1;
}

void removeTrapVamp(LocationID ID, char *num, int **time) {
    num[ID] -= 1;
    time[ID][num[ID]] = 0;
}

void reduceTimeTrapVamp(LocationID ID, char *num, int **time) {

    if (num[ID] == 1) return;

    int i;
    int amountToRemove = 0;
    for (i=num[ID]-1;i>=0;i++) {
        time[ID][i] -= 1;
        if (time[ID][i] == 0) {
            amountToRemove++;
        }
    }

    num[ID] -= amountToRemove;
}

int isIn(int *a, int n, int c) {
    int i;

    for (i=0;i<n;i++) {
        if (a[i] == c) {
            return 0;
        }
    }
    return 1;
}


// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    GameView gameView = malloc(sizeof(struct gameView));
    //strlen() length, loop through
    int pastPlaysSize = strlen(pastplays);
    //Round
    gameView->round = pastPlaysSize/40;
    //Current Player
    gameView->currentPlayer = (pastPlaysSize/8)%5;
    int i;
    int curPlayer = 0;

    char *trapsNum[MAX_MAP_LOCATION] = {0};
    int **trapsTime[MAX_MAP_LOCATION][3] = {0};

    char *vampsNum[MAX_MAP_LOCATION] = {0};
    int **vampsTime[MAX_MAP_LOCATION][3] = {0};

    int trail[TRAIL_SIZE] = {-1};

    int seen[TRAIL_SIZE] = {-1};
    int seenUpto = 0;

    for (i=0;i<5;i++) {
        gameView->upto[i] = 0;
    }

    // Initialise hunter's health
    for (i=0;i<4;i++) {
        gameView->health[i] = 9;
    }

    // Initialise Dracula's health
    gameView->health[PLAYER_DRACULA] = 40;

    for(i=0;i<pastPlaysSize;i+8){
        char *currLocation = {pastPlays[i+1], pastPlays[i+2]};
        strcpy(gameView->path[currPlayer][gameView->upto[curPlayer]], currLocation);

        LocationID currLocationID = abbrevToID(currLocation);

        // If the current player is Dracula
        if (curPlayer == PLAYER_DRACULA) {

            // If < 3 encounters in current city, an encounter can be added
            if (trapsNum[currLocationID] + vampsNum[currLocationID] < 3) {

                // If the current round is divisible by 13 and a vamp, otherwise add a trap
                if ((i/40)%13 == 0) {
                    addTrapVamp(currLocationID, vampsNum, vampsTime);
                } else {
                    addTrapVamp(currLocationID, vampsNum, vampsTime);
                }
            }

            // For each hunter in the same location as Dracula, Dracula loses 10 health
            for (i=0;i<4;i++) {
                if (gameView->path[i][upto[i]] == currLocation) {
                    gameView->health[PLAYER_DRACULA] -= 10;
                }
            }

            // If Dracula is at sea he loses 2 health
            if (idToType(currLocationID) == SEA) {
                gameView->health[PLAYER_DRACULA] -= 2;
            }

            // If Dracula is at Castle Dracula he gains 10 health
            if (currLocationID == CASTLE_DRACULA) {
                gameView->health[PLAYER_DRACULA] += 10;
            }
        } else {
            // Process the other players

            // Increase health if they rest (cap it at 9)
            if (gameView->upto[currPlayer] > 1) {
                if (gameView->path[currPlayer][(gameView->upto[currPlayer])-1] == currLocation) {
                    gameView->health[currPlayer] += 3;
                }

                if (gameView->health[currPlayer] > 9) {
                    gameView->health[currPlayer] = 9;
                }
            }

            // Check if a trap is encountered
            if (trapsNum[currLocationID] > 0) {
                while (trapsNum[currLocationID] > 0 || gameView->health[currPlayer] > 0) {
                    removeTrapVamp(currLocationID, trapsNum, trapsTime);
                    gameView->health[currPlayer] -= 2;
                }
            }

            // If their health is < 0 then they can't do anything else, if not then check for
            // an encounter with Dracula
            if (gameView->health[currPlayer] > 0) {
                if (currLocation == gameView->path[PLAYER_DRACULA][gameView->upto[PLAYER_DRACULA]]) {
                    gameView->health[currPlayer] -= 4;
                }
            }
        }

        // Remove any traps/vamps which have expired

        // Get Dracula's trail (reduce time of traps and vamps on his trail)
        getHistory(gameView, PLAYER_DRACULA, trail);


        for (i=0;i<6;i++) {

            // If the current location is valid and has not yet been seen 
            if (trail[i] >= MAX_MAP_LOCATION && trail[i] <= MAX_MAP_LOCATION && 
                !isIn(seen, seenUpto, trail[i]) && trail[i] != -1) {
                
                // Add it to the seen array
                seen[seenUpto] = trail[i];


                // Reduce the time for the traps and vamps
                reduceTimeTrapVamp(trail[i], trapsNum, trapsTime);
                reduceTimeTrapVamp(trail[i], vampsNum, vampsTime);
            }
        }

        curPlayer = (curPlayer+1)%5;
    }

    return gameView;
}


// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    //COMPLETE THIS IMPLEMENTATION
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    //Done feel free to change
    return currentView->round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    //Done feel free to change
    return currentView->currentPlayer;
}

    //Done feel free to change
int getScore(GameView currentView)
{
    //Done feel free to change
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    //Done feel free to change
    return health[player];
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return NULL;
}
