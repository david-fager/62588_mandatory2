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

// Initializes the deck
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
pthread_cond_t revealCond = PTHREAD_COND_INITIALIZER;
int totalCardNumber = 1;

int main() {

    printf("[MAIN] Shuffeling card deck\n");
    shuffleDeck();

    printf("[MAIN] Splitting deck into two halves\n");
    int index = 0;
    for(int i = 0; i < 52; i++) {
        if (i < 26) {
            strcpy(firstHalf[i], deck[i]);
        } else {
            strcpy(secondHalf[index++], deck[i]);
        }
    }

    pthread_t tid1, tid2;
    printf("[MAIN] Creating thread 1 (player one)\n");
    pthread_create(&tid1, NULL, playerOne, NULL);
    printf("[MAIN] Creating thread 2 (player two)\n");
    pthread_create(&tid2, NULL, playerTwo, NULL);

    pthread_join(tid1, NULL);
    printf("[MAIN] Thread 1 (player one) finished\n");
    pthread_join(tid2, NULL);
    printf("[MAIN] Thread 2 (player two) finished\n");

    /*
    for(int i = 0; i < 52; i++) {
        printf("[%d] %s\n", i, deck[i]);
        fflush(stdout);
    }
    */
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
        pthread_mutex_lock(&revealMutex);
        printf("[PLAYER_ONE] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 != 0) {
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;

        printf("[PLAYER_ONE] Signal received, revealing: %s\n", firstHalf[i]);

        //TODO: Lav filehåndtering og regler her

        printf("[PLAYER_ONE] Signaling (unblocking) player two\n");
        pthread_cond_signal(&revealCond);

        printf("[PLAYER_ONE] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
}

void *playerTwo() {
    printf("[PLAYER_TWO] Started\n");

    for (int i = 0; i < 26; i++) {
        pthread_mutex_lock(&revealMutex);
        printf("[PLAYER_TWO] Requested lock on reveal mutex\n");

        if (totalCardNumber % 2 == 0) {
            pthread_cond_wait(&revealCond, &revealMutex);
        }
        totalCardNumber++;

        printf("[PLAYER_TWO] Signal received, revealing: %s\n", secondHalf[i]);

        //TODO: Lav filehåndtering og regler her

        printf("[PLAYER_TWO] Signaling (unblocking) player one\n");
        pthread_cond_signal(&revealCond);

        printf("[PLAYER_TWO] Unlocking reveal mutex\n");
        pthread_mutex_unlock(&revealMutex);
    }
}