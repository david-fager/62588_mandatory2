#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// Functions
void shuffleDeck();
void *playerOne();
void *playerTwo();

// Global variables
char deck[52][100] = {
        "2 (diamond)", "3 (diamond)", "4 (diamond)", "5 (diamond)", "6 (diamond)", "7 (diamond)", "8 (diamond)",
        "9 (diamond)", "10 (diamond)", "jack (diamond)", "queen (diamond)", "king (diamond)", "ace (diamond)",
        "2 (heart)", "3 (heart)", "4 (heart)", "5 (heart)", "6 (heart)", "7 (heart)", "8 (heart)",
        "9 (heart)", "10 (heart)", "jack (heart)", "queen (heart)", "king (heart)", "ace (heart)",
        "2 (spade)", "3 (spade)", "4 (spade)", "5 (spade)", "6 (spade)", "7 (spade)", "8 (spade)",
        "9 (spade)", "10 (spade)", "jack (spade)", "queen (spade)", "king (spade)", "ace (spade)",
        "2 (club)", "3 (club)", "4 (club)", "5 (club)", "6 (club)", "7 (club)", "8 (club)",
        "9 (club)", "10 (club)", "jack (club)", "queen (club)", "king (club)", "ace (club)"
};
char firstHalf[26][100];
char secondHalf[26][100];
pthread_mutex_t revealMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t revealCond = PTHREAD_COND_INITIALIZER;
int totalCardNumber = 1;
FILE *fptr;
fpos_t fpos;
int playerOnePoints = 0, playerTwoPoints = 0;

int main() {

    // Prepares the file, creating if non-existent, emptying if existent
    fptr = fopen("BattleLog.txt", "w");
    fprintf(fptr, "ROUND. PLAYER ONE / PLAYER TWO\n");
    if (fptr == NULL) {
        printf("[MAIN] FILE ERROR OCCURED");
        fflush(stdout);
        exit(-1);
    }
    fclose(fptr);

    // Shuffeling the deck
    printf("[MAIN] Shuffeling card deck\n");
    shuffleDeck();

    // Splitting deck into two halves
    printf("[MAIN] Splitting deck into two halves\n");
    int index = 0;
    for(int i = 0; i < 52; i++) {
        if (i < 26) {
            strcpy(firstHalf[i], deck[i]);
        } else {
            strcpy(secondHalf[index++], deck[i]);
        }
    }

    // Creating threads
    pthread_t tid1, tid2;
    printf("[MAIN] Creating thread 1 (player one)\n");
    pthread_create(&tid1, NULL, playerOne, NULL);
    printf("[MAIN] Creating thread 2 (player two)\n");
    pthread_create(&tid2, NULL, playerTwo, NULL);

    // Joining threads
    pthread_join(tid1, NULL);
    printf("[MAIN] Thread 1 (player one) finished\n");
    pthread_join(tid2, NULL);
    printf("[MAIN] Thread 2 (player two) finished\n");

    // Writes to stdout and file who won the game
    fptr = fopen("BattleLog.txt", "a");
    if (playerOnePoints > playerTwoPoints) {
        fprintf(fptr, "\nPLAYER ONE WON WITH %d POINTS OVER %d\n", playerOnePoints, playerTwoPoints);
        printf("GAME ENDED, PLAYER ONE WON WITH %d POINTS OVER %d\n", playerOnePoints, playerTwoPoints);
    } else if (playerTwoPoints > playerOnePoints) {
        fprintf(fptr, "\nPLAYER TWO WON WITH %d POINTS OVER %d\n", playerTwoPoints, playerOnePoints);
        printf("GAME ENDED, PLAYER TWO WON WITH %d POINTS OVER %d\n", playerTwoPoints, playerOnePoints);
    } else {
        fprintf(fptr, "\nIT IS A TIE AT %d / %d\n", playerTwoPoints, playerOnePoints);
        printf("GAME ENDED, IT IS A TIE AT %d / %d\n", playerOnePoints, playerTwoPoints);
    }

    return 0;
}

// Shuffles the sorted card deck after its initialization
void shuffleDeck() {
    char temp[100];

    // Specifies a randomizing seed, so rand() functions properly
    srand(time(NULL));

    // Swaps each card in the deck with a random card through the 'rand()' function
    for(int i = 0; i < 52; i++) {
        int cardNumber = rand() % 52;
        strcpy(temp, deck[i]);
        strcpy(deck[i], deck[cardNumber]);
        strcpy(deck[cardNumber], temp);
    }
}

void *playerOne() {
    printf("[PLAYER_ONE] Started\n");
    usleep(250000);

    for (int i = 0; i < 26; i++) {

        // Locks the mutex
        pthread_mutex_lock(&revealMutex);
        printf("[PLAYER_ONE] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 == 0) {
            // If above condition is met (totalCardNumber is uneven), the below method
            // blocks this thread and unlocks the mutex, until another thread sends a signal
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;
        printf("\nROUND %d HAS BEGUN\n", i + 1);

        // The card is revealed and handled in the file
        printf("[PLAYER_ONE] Signal received, revealing: %s\n", firstHalf[i]);
        printf("[PLAYER_ONE] Dealing with file\n");
        fptr = fopen("BattleLog.txt", "a");
        if (fptr == NULL) {
            printf("[PLAYER_ONE] FILE ERROR OCCURED\n");
            fflush(stdout);
            exit(-1);
        }
        // Get the current position of curser in file and write the revealed card to the file
        fgetpos(fptr, &fpos);
        fprintf(fptr, "%d. %s\n", i + 1, firstHalf[i]);
        fclose(fptr);

        // A signal to a potentially waiting player two is sent
        printf("[PLAYER_ONE] Signaling (unblocking) player two\n");
        pthread_cond_signal(&revealCond);

        // Unlocking the mutex allows for the other player to lock the mutex again and continue (wake up)
        printf("[PLAYER_ONE] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
    return NULL;
}

void *playerTwo() {
    printf("[PLAYER_TWO] Started\n");
    usleep(250000);

    for (int i = 0; i < 26; i++) {
        pthread_mutex_lock(&revealMutex);
        printf("[PLAYER_TWO] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 != 0) {
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;

        // Card revealed and copied into savedhand
        printf("[PLAYER_TWO] Signal received, revealing: %s\n", secondHalf[i]);
        char savedHand[50];
        strcpy(savedHand, secondHalf[i]);

        char tempHand[50];
        char fullHand[50];
        char * tokenizedHand;
        char playerOneChar[50];
        // opens file and sets the position to start of player one's revealed card,
        // then reads the card and tokenizes it, to see its value
        fptr = fopen("BattleLog.txt", "r+");
        fsetpos(fptr, &fpos);
        fgets(tempHand, 50, fptr);
        strcpy(fullHand, tempHand);
        strtok(fullHand, "\n");
        tokenizedHand = strtok(tempHand, " ");

        // The actual tokenizing and value assignment
        int index = 0;
        int p1CardValue = 0, p2CardValue = 0;
        while (tokenizedHand != NULL) {
            if (index++ == 1) {
                strcpy(playerOneChar, tokenizedHand);
                if (strcmp(tokenizedHand, "jack") == 0) {
                    p1CardValue = 11;
                } else if (strcmp(tokenizedHand, "queen") == 0) {
                    p1CardValue = 12;
                } else if (strcmp(tokenizedHand, "king") == 0) {
                    p1CardValue = 13;
                } else if (strcmp(tokenizedHand, "ace") == 0) {
                    p1CardValue = 14;
                } else {
                    // Atoi gets the int value from the pointer
                    p1CardValue = atoi(tokenizedHand);
                }
                if (strcmp(strtok(secondHalf[i], " "), "jack") == 0) {
                    p2CardValue = 11;
                } else if (strcmp(strtok(secondHalf[i], " "), "queen") == 0) {
                    p2CardValue = 12;
                } else if (strcmp(strtok(secondHalf[i], " "), "king") == 0) {
                    p2CardValue = 13;
                } else if (strcmp(strtok(secondHalf[i], " "), "ace") == 0) {
                    p2CardValue = 14;
                } else {
                    p2CardValue = atoi(strtok(secondHalf[i], " "));
                }
            }
            // Gets next in tokenization
            tokenizedHand = strtok(NULL, " ");
        }

        // Start on string handling, fullhand is becoming: "[player one's card] / [player two's card]"
        fsetpos(fptr, &fpos);
        strcat(fullHand, " / ");
        strcat(fullHand, savedHand);

        // Depending on which player has the higher card value, it prepares
        // the string going into the file and increments the player's amount of points
        if (p1CardValue > p2CardValue) {
            strcat(fullHand, " - ");
            strcat(fullHand, playerOneChar);
            strcat(fullHand, " greater than ");
            strcat(fullHand, strtok(secondHalf[i], " "));
            strcat(fullHand, ", 1 point to player one");
            playerOnePoints++;
            printf("[INFO] PLAYER ONE GOT THE POINT\n");
        } else if (p2CardValue > p1CardValue) {
            strcat(fullHand, " - ");
            strcat(fullHand, strtok(secondHalf[i], " "));
            strcat(fullHand, " greater than ");
            strcat(fullHand, playerOneChar);
            strcat(fullHand, ", 1 point to player two");
            playerTwoPoints++;
            printf("[INFO] PLAYER TWO GOT THE POINT\n");
        } else {
            strcat(fullHand, " - cards of same value, tied");
            printf("[INFO] PLAYERS HAD SAME CARD, NO POINTS\n");
        }

        // The full string is printed to the file.
        // String is along: "[player one's card] / [player two's card] - [who got the point]"
        fprintf(fptr, "%s\n", fullHand);
        fclose(fptr);

        printf("[PLAYER_TWO] Signaling (unblocking) player one\n");
        pthread_cond_signal(&revealCond);

        printf("[PLAYER_TWO] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
    return NULL;
}