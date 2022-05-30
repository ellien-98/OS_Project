#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#define _XOPEN_SOURCE 500
#include "3200248_3150094_3180129.h"

int Ncust;
unsigned int seed;
int newSeatArrayA[NzoneA][Nseat];
int newSeatArrayB[NzoneB][Nseat];
int pay_chance = 0;
// int zone_chance = 0;
int ticketcost = 0;
int balance = 0;
int id_transaction = 0;
double totalWaitingTime = 0; // mesos xronos anamonhs
double totalSupportTime = 0; // mesos xronos eksipirethshs pelaton
int currentTelInUse = 0;
int currentCashInUse = 0;
int isProcessingSeatsStarted = 0;
int isProcessingSeatsFinished = 0;
int isProcessingSeats = 0;
int successfulPayments = 0; // to compare for final statistics
int unsuccessfulPayments = 0;
int reserveSeatsResult = 500;
int choice = 50;
float creditCancellation = 0;
float seatCancellation = 0;
// float amountSeatCancels = 0.0f;
// float amountCreditCancels = 0.0f;
float successfulReservs = 0.0f;
char zone = 'x';

pthread_mutex_t TelCounter;
pthread_mutex_t CashCounter;
pthread_mutex_t seatsMutex;
pthread_mutex_t PrintMutex;

pthread_mutex_t ticketFinder;
pthread_mutex_t addToBalance;
pthread_mutex_t waitingTimeMutex;
pthread_mutex_t supportTimeMutex;

pthread_cond_t telThresholdCond;
pthread_cond_t cashThresholdCond;
pthread_cond_t ticketThresholdCond;
pthread_cond_t balanceThresholdCond;
pthread_cond_t printThresholdCond;
pthread_cond_t timeThresholdCond;
pthread_cond_t seatsThresholdCond;

int rand_r(unsigned int *seed);

// returns 1 for zone A, 0 for zone B
int calcPzoneA()
{
    int zoneA = 1, zoneB = 0;
    srand(time(NULL));
    float number = rand() % 100 + 1; // Generate random number 1 to 100
    if (number <= 70)                // 70% chance for zone B
        return zoneB;                // aka 0
    else
        return zoneA; // aka 1
}
int calcPCardSuccess()
{

    srand(time(NULL));
    float number = rand() % 100 + 1; // Generate random number 1 to 100
    if (number <= 90)                // 90% chance for successful payment
        return 1;
    else
        return -1;
}

// probably not necessary
int rndGen(int low, int high)
{
    int result; // first seed the random number generator
    /*
     * srand(seed);    //user gave the seed
    result= ( (rand() % (high+1-low) ) + low );
     */
    // rand_r(&seed);    //user gave the seed
    result = ((rand_r(&seed) % (high + 1 - low)) + low);
    return result;
}

int reserveSeatsZA(int tickets, void *tId)
{
    int *id = (int *)tId;
    int rc, i, j, k, seatsAvail = 0, seatsCounter, startSeatPosition; //k;

    isProcessingSeats = 1;
    for (i = 0; i < NzoneA; i++)
    {
        for (j = 0; j < Nseat; j++)
        {
            if (newSeatArrayA[i][j] == -1)
                seatsAvail++;
        }
    }

    if (seatsAvail == 0)
    {
        printf("ERROR: The seats in requested zone are sold out, goodbye\n");
        return -1;
    }

    printf("Customer %d :  %d tickets\n", *id, tickets);
    if (seatsAvail >= tickets)
    {
        for (i = 0; i < NzoneA; i++)
        {
            seatsCounter = 0;
            for (j = 0; j < Nseat; j++)
            {
                if (newSeatArrayA[i][j] != -1)
                {
                    seatsCounter = 0;
                }
                else
                {
                    seatsCounter++;
                    if (seatsCounter == tickets)
                    {
                        startSeatPosition = (j + 1) - tickets; //dhlwsi k
                        for (int k = 0; k < tickets; k++)
                        {
                            newSeatArrayA[i][startSeatPosition + k] = *id;
                        }
                        printf("Seira %d, theseis %d ews %d in Zone B\n", i, startSeatPosition + 1, startSeatPosition + tickets);
                        return 1;
                    }
                }
            }
        }
    }
    printf("There are not available continuous seats in Zone A \n");
    return -2;
}
int reserveSeatsZB(int tickets, void *tId)
{
    int *id = (int *)tId;
    int rc, i, j, k, seatsAvail = 0, seatsCounter, startSeatPosition; //k;

    isProcessingSeats = 1;
    for (i = 0; i < NzoneB; i++)
    {
        for (j = 0; j < Nseat; j++)
        {
            if (newSeatArrayB[i][j] == -1)
                seatsAvail++;
        }
    }

    if (seatsAvail == 0)
    {
        printf("ERROR: The seats in requested zone are sold out, goodbye\n");
        return -1;
    }

    printf("Customer %d :  %d tickets\n", *id, tickets);
    if (seatsAvail >= tickets)
    {
        for (i = 0; i < NzoneB; i++)
        {
            seatsCounter = 0;
            for (j = 0; j < Nseat; j++)
            {
                if (newSeatArrayB[i][j] != -1)
                {
                    seatsCounter = 0;
                }
                else
                {
                    seatsCounter++;
                    if (seatsCounter == tickets)
                    {
                        startSeatPosition = (j + 1) - tickets;
                        for (int k = 0; k < tickets; k++)
                        {
                            newSeatArrayB[i][startSeatPosition + k] = *id;
                        }
                        printf("Seira %d, theseis %d ews %d in Zone B\n", i, startSeatPosition + 1, startSeatPosition + tickets);
                        return 1;
                    }
                }
            }
        }
    }
    printf("There are not available continuous seats in Zone B \n");
    return -2;
}

void *customerServe(void *tId)
{
    // struct contains two member variables:
    // tv_sec – The variable of the time_t type made to store time in seconds.
    // tv_nsec – The variable of the long type used to store time in nanoseconds.
    struct timespec start, telConnect, custCompleted; // stop for error stop, completed for successful reservation
    struct timespec cashConnect, telDisconnect;
    // struct timespec custStopWait;
    struct timespec start2, stop2, stop;
    int rndSeats;
    // int rndZone;
    int rndSecTel;
    int rndSecCash;
    int *threadId = (int *)tId;
    int rc;
    int totalSeatsCost = 0;
    int flagPayment = 0;
    int zoneSelection = -10;

    // begin counting support time for this thread
    clock_gettime(CLOCK_REALTIME, &start);

    rc = pthread_mutex_lock(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }
    while (currentTelInUse >= NTEL)
    { // avoid spurious wakeup
        printf("Currently waiting for the first available Telephonist, please wait, %d ---\n", currentTelInUse);
        rc = pthread_cond_wait(&telThresholdCond, &TelCounter);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }
    }
    printf("An available telephonist is found for customer %d\n", *threadId);
    currentTelInUse++;
    // begin counting time the customer is being serviced - talking on the phone
    clock_gettime(CLOCK_REALTIME, &telConnect);
    // add this customer's waiting time until telephonist support, to total
    totalWaitingTime = totalWaitingTime + (telConnect.tv_nsec - start.tv_nsec);

    currentTelInUse--;
    rc = pthread_mutex_unlock(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_cond_signal(&telThresholdCond);  //broadcast or signal ?
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_signal(telThresholdCond) is %d\n", rc);
        pthread_exit(&rc);
    }

    zoneSelection = calcPzoneA(); // 1 for A, 0 for B

    // select random number of tickets the client wants
    rndSeats = rndGen(N_SEAT_LOW, N_SEAT_HIGH);
    // time the telephonist needs to determine availability of continuous seats in selected zone
    int rndSeatAvailTime = rndGen(T_SEAT_LOW, T_SEAT_HIGH);
    sleep(rndSeatAvailTime); // thread goes sleeping for this time

    rc = pthread_mutex_lock(&seatsMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock(&seatsMutex) is %d\n", rc);
        pthread_exit(&rc);
    }
    // avoid spurious wakeup
    while (isProcessingSeats == 1)
    {
        printf("Another thread is processing the theater, please wait, (from thread %d)\n", *threadId);
        rc = pthread_cond_wait(&seatsThresholdCond, &seatsMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_cond_wait(seatsThresholdCond) is %d\n", rc);
            pthread_exit(&rc);
        }
    }
    if (zoneSelection == 1)
    {
        reserveSeatsResult = reserveSeatsZA(rndSeats, tId);
    }
    else
    {
        reserveSeatsResult = reserveSeatsZB(rndSeats, tId);
    }
    if (reserveSeatsResult == -1)
    {
        printf("Customer %d: The reservation failed not continuous seats found. \n", *threadId);
        seatCancellation++;
        clock_gettime(CLOCK_REALTIME, &custCompleted);
        rc = pthread_mutex_lock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_cond_wait(waitingTimeMutex) is %d\n", rc);
            pthread_exit(&rc);
        }
        totalSupportTime = totalSupportTime + (custCompleted.tv_nsec - start.tv_nsec);
        rc = pthread_mutex_unlock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(supportTimeMutex) is %d\n", rc);
            pthread_exit(&rc);
        }
    }
    else if (reserveSeatsResult == -2)
    {
        printf("Customer %d: The reservation failed not continuous seats found. \n", *threadId);
        seatCancellation++;
        clock_gettime(CLOCK_REALTIME, &custCompleted);
        rc = pthread_mutex_lock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_cond_wait(waitingTimeMutex) is %d\n", rc);
            pthread_exit(&rc);
        }
        totalSupportTime = totalSupportTime + (custCompleted.tv_nsec - start.tv_nsec);
        rc = pthread_mutex_unlock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(supportTimeMutex) is %d\n", rc);
            pthread_exit(&rc);
        }
    }
    else if (reserveSeatsResult == 1)
    { // there are enough available seats
        totalSeatsCost = zoneSelection == 1 ? rndSeats * CzoneA : rndSeats * CzoneB;
        printf("The seats you requested are available, total cost: %d€\n", totalSeatsCost);
        flagPayment = 1;
        clock_gettime(CLOCK_REALTIME, &telDisconnect);

    } // end seatA reservation

    isProcessingSeats = 0;
    rc = pthread_cond_signal(&seatsThresholdCond);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_broadcast(&seatsThresholdCond) is %d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_unlock(&seatsMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock(&seatsMutex) is %d\n", rc);
        pthread_exit(&rc);
    }
    // end of this customer.Thread finishes
    if (reserveSeatsResult != 1)
        pthread_exit(threadId);

    if (flagPayment == 1)
    { // go for cashier
        rc = pthread_mutex_lock(&CashCounter);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        } // spurious wake up
        while (currentCashInUse >= NCASH)
        {
            printf("Currently waiting for the first available Cashier, please wait, %d ---\n", currentCashInUse);
            rc = pthread_cond_wait(&cashThresholdCond, &CashCounter);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_cond_wait(cashThresholdCond) is %d\n", rc);
                pthread_exit(&rc);
            }
        }
        printf("An available Cashier is found for customer %d\n", *threadId);
        currentCashInUse++;

        clock_gettime(CLOCK_REALTIME, &cashConnect);

        rndSecCash = rndGen(T_CASH_LOW, T_CASH_HIGH);
        sleep(rndSecCash);

        currentCashInUse--;
        rc = pthread_cond_signal(&cashThresholdCond);
        rc = pthread_mutex_unlock(&CashCounter);

        choice = calcPCardSuccess();
        if (choice == 1)
        { // tha ginei h pliromh

            rc = pthread_mutex_lock(&addToBalance);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_mutex_lock(addToBalance) is %d\n", rc);
                pthread_exit(&rc);
            }
            balance = balance + totalSeatsCost;
            successfulPayments++;
            clock_gettime(CLOCK_REALTIME, &custCompleted);
            printf("Customer %d successfull payment %d€\n", *threadId, totalSeatsCost);
            rc = pthread_mutex_unlock(&addToBalance);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_mutex_unlock(addToBalance) is %d\n", rc);
                pthread_exit(&rc);
            }
        }
        else if (choice == -1)
        { // den tha ginei h pliromi
            printf("Customer %d: The reservation failed because the credit card transaction was not accepted\n", *threadId);
            // unsuccessfulPayments++;
            clock_gettime(CLOCK_REALTIME, &custCompleted);
            flagPayment = 0;
            creditCancellation++;
            pthread_exit(&threadId);
        }

        // totalsupporttime
        rc = pthread_mutex_lock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(CashCounter) is %d\n", rc);
            pthread_exit(&rc);
        }
        totalSupportTime += custCompleted.tv_nsec - start.tv_nsec;
        rc = pthread_mutex_unlock(&supportTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(CashCounter) is %d\n", rc);
            pthread_exit(&rc);
        }

        // total waiting time
        rc = pthread_mutex_lock(&waitingTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(CashCounter) is %d\n", rc);
            pthread_exit(&rc);
        }
        totalWaitingTime += cashConnect.tv_nsec - telDisconnect.tv_nsec;
        rc = pthread_mutex_unlock(&waitingTimeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(CashCounter) is %d\n", rc);
            pthread_exit(&rc);
        }
    }
    else if (flagPayment == 0)
    { // cancel reserved seats
        if (zoneSelection == 1)
        {
            for (int i = 0; i < NzoneA; i++)
            {
                for (int j = 0; j < Nseat; j++)
                {
                    if (newSeatArrayA[i][j] == *threadId)
                    {
                        newSeatArrayA[i][j] = -1;
                    }
                }
            }
        }
        else if (zoneSelection == 0)
        {
            for (int i = 0; i < NzoneB; i++)
            {
                for (int j = 0; j < Nseat; j++)
                {
                    if (newSeatArrayB[i][j] == *threadId)
                    {
                        newSeatArrayB[i][j] = -1;
                    }
                }
            }
        }
    }
    pthread_exit(tId);
}

int main(int argc, char *argv[])
{

    int rc, i, j;
    // Checks if user gave the correct input
    if (argc != 3)
    {
        printf("ERROR: the program should take two arguments, the number of customers to create and the seed number!\n");
        exit(0);
    }
    // convert input char* to int
    Ncust = atoi(argv[1]);
    seed = atoi(argv[2]);

    // Check if the input values are positive numbers, otherwise end program
    if (seed < 0)
    {
        printf("ERROR: the number of seed should be a positive number. Current number given %d.\n", seed);
        exit(-1);
    }
    if (Ncust <= 0)
    {
        printf("ERROR: the number of customers should be a positive number. Current number given %d.\n", Ncust);
        exit(-1);
    }

    for (i = 0; i < NzoneA; i++)
    {
        for (j = 0; j < Nseat; j++)
        {
            newSeatArrayA[i][j] = -1;
        }
    }
    for (i = 0; i < NzoneB; i++)
    {
        for (j = 0; j < Nseat; j++)
        {
            newSeatArrayB[i][j] = -1;
        }
    }
    /*arxikopoiisi tou mutex kai tou condition*/
    rc = pthread_mutex_init(&TelCounter, NULL);
    rc = pthread_mutex_init(&CashCounter, NULL);
    rc = pthread_mutex_init(&seatsMutex, NULL);
    rc = pthread_mutex_init(&PrintMutex, NULL); // den exw xrhsimopoihsei printmutex sthn customerServe
    rc = pthread_mutex_init(&ticketFinder, NULL);
    rc = pthread_mutex_init(&addToBalance, NULL);
    rc = pthread_mutex_init(&waitingTimeMutex, NULL);
    rc = pthread_mutex_init(&supportTimeMutex, NULL);

    rc = pthread_cond_init(&telThresholdCond, NULL);
    rc = pthread_cond_init(&cashThresholdCond, NULL);
    rc = pthread_cond_init(&ticketThresholdCond, NULL);
    rc = pthread_cond_init(&balanceThresholdCond, NULL);
    rc = pthread_cond_init(&printThresholdCond, NULL);
    rc = pthread_cond_init(&timeThresholdCond, NULL);
    rc = pthread_cond_init(&seatsThresholdCond, NULL);

    pthread_t *threads = malloc(sizeof(pthread_t) * Ncust);
    if (threads == NULL)
    {
        printf("ERROR: Failed to allocate threads, not enough memory!\n");
        return -1;
    }
    printf("Main: Creating %d threads, one for each customer.\n", Ncust);
    int threadIds[Ncust];
    for (int i = 0; i < Ncust; i++)
    {
        threadIds[i] = i;
        // MARIA MOU edw sto allaksa evala i anti gia i+1, IDKW
        rc = pthread_create(&threads[i], NULL, customerServe, &threadIds[i]);
        // wait for random seconds in [treslow, treshigh] before going to next thread creation
        int rndslpTime = rndGen(T_RES_LOW, T_RES_HIGH);
        sleep(rndslpTime);
    }

    // the end, wait for costumer threads to end
    void *status;
    for (int i = 0; i < Ncust; i++)
    {
        rc = pthread_join(threads[i], &status);
    }
    /*"katastrofi" mutex kai condition*/
    rc = pthread_mutex_destroy(&TelCounter);
    rc = pthread_mutex_destroy(&CashCounter);
    rc = pthread_mutex_destroy(&seatsMutex);
    rc = pthread_mutex_destroy(&PrintMutex);
    rc = pthread_mutex_destroy(&ticketFinder);
    rc = pthread_mutex_destroy(&addToBalance);
    rc = pthread_mutex_destroy(&waitingTimeMutex);
    rc = pthread_mutex_destroy(&supportTimeMutex);

    rc = pthread_cond_destroy(&telThresholdCond);
    rc = pthread_cond_destroy(&cashThresholdCond);
    rc = pthread_cond_destroy(&ticketThresholdCond);
    rc = pthread_cond_destroy(&balanceThresholdCond);
    rc = pthread_cond_destroy(&printThresholdCond);
    rc = pthread_cond_destroy(&timeThresholdCond);
    rc = pthread_cond_destroy(&seatsThresholdCond);

    printf("\nZone A\n");
    for (int i = 0; i < NzoneA; i++)
    { // ana seira
        for (int j = 0; j < Nseat; j++)
        { // ana seira
            printf(" Ζώνη Α / Σειρά %d / Θέση %d / Πελάτης %d, ", i + 1, j + 1, newSeatArrayA[i][j]);
        }
    }
    printf("\nZone B\n");
    for (int i = 0; i < NzoneB; i++)
    {
        for (int j = 0; j < Nseat; j++)
        { // ana seira
            printf(" Ζώνη B / Σειρά %d / Θέση %d / Πελάτης %d, ", i + 1, j + 1, newSeatArrayB[i][j]);
        }
    }

    printf("\nThe balance is: %d\n", balance);
    // printf("Total transactions: %d\n",id_transaction);							//Sunolika Esoda
    printf("Average waiting Time: %f\n", totalWaitingTime / Ncust); // Mesos xronos anamwnhs pelatwn
    printf("Average serving: %f\n", totalSupportTime / Ncust);      // mesos xronos ejhpiretisis pelatwn
    printf("Amount of transactions cancelled because of lack of seats: %f\n", seatCancellation / Ncust);
    printf("Amount of transactions cancelled because of credit card failure: %f\n", creditCancellation / Ncust);
    successfulReservs = Ncust - seatCancellation - creditCancellation;
    printf("Amount of successful transactions: %f\n", successfulReservs / Ncust);

    return 1;
}
