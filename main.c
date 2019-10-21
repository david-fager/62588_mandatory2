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
char deck[52][20] = {
        "2 (diamond)", "3 (diamond)", "4 (diamond)", "5 (diamond)", "6 (diamond)", "7 (diamond)", "8 (diamond)",
        "9 (diamond)", "10 (diamond)", "jack (diamond)", "queen (diamond)", "king (diamond)", "ace (diamond)",
        "2 (heart)", "3 (heart)", "4 (heart)", "5 (heart)", "6 (heart)", "7 (heart)", "8 (heart)",
        "9 (heart)", "10 (heart)", "jack (heart)", "queen (heart)", "king (heart)", "ace (heart)",
        "2 (spade)", "3 (spade)", "4 (spade)", "5 (spade)", "6 (spade)", "7 (spade)", "8 (spade)",
        "9 (spade)", "10 (spade)", "jack (spade)", "queen (spade)", "king (spade)", "ace (spade)",
        "2 (club)", "3 (club)", "4 (club)", "5 (club)", "6 (club)", "7 (club)", "8 (club)",
        "9 (club)", "10 (club)", "jack (club)", "queen (club)", "king (club)", "ace (club)"
};
char firstHalf[26][20];
char secondHalf[26][20];
pthread_mutex_t revealMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t revealCond = PTHREAD_COND_INITIALIZER;
int totalCardNumber = 1;
FILE *fPtr1, *fPtr2;

int main() {
    // Creates/empties the file representing the player's hands
    fPtr1 = fopen("playerOneHand.txt", "w");
    fPtr2 = fopen("playerTwoHand.txt", "w");
    if (fPtr1 == NULL || fPtr2 == NULL) {
        printf("[MAIN] FILE ERROR OCCURED");
        fflush(stdout);
        exit(-1);
    }
    fclose(fPtr1);
    fclose(fPtr2);

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

    return 0;
}

// Shuffles the sorted card deck after its initialization
void shuffleDeck() {
    char temp[20];

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

    for (int i = 0; i < 26; i++) {
        // Locks the mutex
        pthread_mutex_lock(&revealMutex);
        printf("\n[PLAYER_ONE] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 == 0) {
            // If above condition is met (totalCardNumber is uneven), the below method
            // blocks this thread and unlocks the mutex, until another thread sends a signal
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;

        // The card is revealed and handled in the file representing their hand
        printf("[PLAYER_ONE] Signal received, revealing: %s\n", firstHalf[i]);

        printf("[PLAYER_ONE] Dealing with file(s)\n");
        fPtr1 = fopen("playerOneHand.txt", "a+");
        if (fPtr1 == NULL) {
            printf("[PLAYER_ONE] FILE ERROR OCCURED");
            fflush(stdout);
            exit(-1);
        }
        fprintf(fPtr1, "%s\n", firstHalf[i]);
        fclose(fPtr1);
        //TODO: Lav filehåndtering og regler her

        // A signal to a potentially waiting player two is sent
        printf("[PLAYER_ONE] Signaling (unblocking) player two\n");
        pthread_cond_signal(&revealCond);

        // Unlocking the mutex allows for the other player to lock the mutex again and continue (wake up)
        printf("[PLAYER_ONE] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
    return NULL;
}

// This function works in the same way as described in the playerOne function
void *playerTwo() {
    printf("[PLAYER_TWO] Started\n");

    for (int i = 0; i < 26; i++) {
        pthread_mutex_lock(&revealMutex);
        printf("[PLAYER_TWO] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 != 0) {
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;

        printf("[PLAYER_TWO] Signal received, revealing: %s\n", secondHalf[i]);
        sleep(1);

        printf("[PLAYER_TWO] Dealing with file(s)\n");
        fPtr2 = fopen("playerTwoHand.txt", "a+");
        if (fPtr2 == NULL) {
            printf("[PLAYER_TWO] FILE ERROR OCCURED");
            fflush(stdout);
            exit(-1);
        }
        fprintf(fPtr2, "%s\n", secondHalf[i]);
        fclose(fPtr2);
        //TODO: Lav filehåndtering og regler her

        printf("[PLAYER_TWO] Signaling (unblocking) player one\n");
        pthread_cond_signal(&revealCond);

        printf("[PLAYER_TWO] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
    return NULL;
}