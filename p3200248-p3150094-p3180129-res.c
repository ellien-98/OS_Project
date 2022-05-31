#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#define _XOPEN_SOURCE 500
#include "p3200248-p3150094-p3180129-res.h"

int Ncust;
unsigned int seed;
int newSeatArrayA[NzoneA][Nseat];
int newSeatArrayB[NzoneB][Nseat];
int pay_chance = 0;
// int zone_chance = 0;
int ticketcost = 0;
int balance = 0;
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
int creditCancellation = 0;
int seatCancellation = 0;
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
    int rc, i, j, k, seatsAvail = 0, seatsCounter, startSeatPosition; // k;

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
                        startSeatPosition = (j + 1) - tickets; // dhlwsi k
                        for (int k = 0; k < tickets; k++)
                        {
                            newSeatArrayA[i][startSeatPosition + k] = *id;
                        }
                        printf("Seira %d, theseis %d ews %d in Zone B\n", i + 1, startSeatPosition + 1, startSeatPosition + tickets);
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
    int rc, i, j, k, seatsAvail = 0, seatsCounter, startSeatPosition; // k;

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
                        printf("Seira %d, theseis %d ews %d in Zone B\n", i + 1, startSeatPosition + 1, startSeatPosition + tickets);
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
    assert(rc == 0);

    while (currentTelInUse >= NTEL)
    { // avoid spurious wakeup
        printf("Currently waiting for the first available Telephonist, please wait, %d ---\n", currentTelInUse);
        rc = pthread_cond_wait(&telThresholdCond, &TelCounter);
        assert(rc == 0);
    }
    printf("An available telephonist is found for customer %d\n", *threadId);
    currentTelInUse++;
    // begin counting time the customer is being serviced - talking on the phone
    clock_gettime(CLOCK_REALTIME, &telConnect);
    // add this customer's waiting time until telephonist support, to total
    rc = pthread_mutex_lock(&waitingTimeMutex);
    assert(rc == 0);
    totalWaitingTime = totalWaitingTime + (telConnect.tv_nsec - start.tv_nsec);
    rc = pthread_mutex_unlock(&waitingTimeMutex);
    assert(rc == 0);


    currentTelInUse--;
    rc = pthread_mutex_unlock(&TelCounter);
    assert(rc == 0);

    rc = pthread_cond_signal(&telThresholdCond);
    assert(rc == 0);

    zoneSelection = calcPzoneA(); // 1 for A, 0 for B

    // select random number of tickets the client wants
    rndSeats = rndGen(N_SEAT_LOW, N_SEAT_HIGH);
    // time the telephonist needs to determine availability of continuous seats in selected zone
    int rndSeatAvailTime = rndGen(T_SEAT_LOW, T_SEAT_HIGH);
    printf("Customer %d: Finish  with Telephonist wait %d seconds\n",*threadId,rndSeatAvailTime);
    sleep(rndSeatAvailTime); // thread goes sleeping for this time

    rc = pthread_mutex_lock(&seatsMutex);
    assert(rc == 0);

    // avoid spurious wakeup
    while (isProcessingSeats == 1)
    {
        printf("Another thread is processing the theater, please wait, (from thread %d)\n", *threadId);
        rc = pthread_cond_wait(&seatsThresholdCond, &seatsMutex);
        assert(rc == 0);
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
        rc = pthread_mutex_lock(&PrintMutex);
        assert(rc == 0);
        printf("Customer %d: The reservation failed not continuous seats found. \n", *threadId);
        rc = pthread_mutex_unlock(&PrintMutex);
        assert(rc == 0);

        seatCancellation++;

        clock_gettime(CLOCK_REALTIME, &custCompleted);
        rc = pthread_mutex_lock(&supportTimeMutex);
        assert(rc == 0);
        totalSupportTime = totalSupportTime + (custCompleted.tv_nsec - start.tv_nsec);
        rc = pthread_mutex_unlock(&supportTimeMutex);
        assert(rc == 0);
    }
    else if (reserveSeatsResult == -2)
    {

        rc = pthread_mutex_lock(&PrintMutex);
        assert(rc == 0);
        printf("Customer %d: The reservation failed not continuous seats found. \n", *threadId);
        rc = pthread_mutex_unlock(&PrintMutex);
        assert(rc == 0);


        seatCancellation++;

        clock_gettime(CLOCK_REALTIME, &custCompleted);
        rc = pthread_mutex_lock(&supportTimeMutex);
        assert(rc == 0);
        totalSupportTime = totalSupportTime + (custCompleted.tv_nsec - start.tv_nsec);
        rc = pthread_mutex_unlock(&supportTimeMutex);
        assert(rc == 0);
    }
    else if (reserveSeatsResult == 1)
    { // there are enough available seats
        totalSeatsCost = zoneSelection == 1 ? rndSeats * CzoneA : rndSeats * CzoneB;

        rc = pthread_mutex_lock(&PrintMutex);
        assert(rc == 0);
        printf("The seats you requested are available, total cost: %d€\n", totalSeatsCost);
        rc = pthread_mutex_unlock(&PrintMutex);
        assert(rc == 0);

        flagPayment = 1;
        clock_gettime(CLOCK_REALTIME, &telDisconnect);

    } // end seatA reservation

    isProcessingSeats = 0;
    rc = pthread_cond_signal(&seatsThresholdCond);
    assert(rc == 0);

    rc = pthread_mutex_unlock(&seatsMutex);
    assert(rc == 0);

    // end of this customer.Thread finishes
    if (reserveSeatsResult != 1)
        pthread_exit(tId);

    if (flagPayment == 1)
    { // go for cashier
        rc = pthread_mutex_lock(&CashCounter);
        assert(rc == 0);
        // spurious wake up
        while (currentCashInUse >= NCASH)
        {
            printf("Currently waiting for the first available Cashier, please wait, %d ---\n", currentCashInUse);
            rc = pthread_cond_wait(&cashThresholdCond, &CashCounter);
            assert(rc == 0);
        }
        printf("An available Cashier is found for customer %d\n", *threadId);
        currentCashInUse++;

        clock_gettime(CLOCK_REALTIME, &cashConnect);

        rc = pthread_mutex_lock(&waitingTimeMutex);
        assert(rc == 0);
        totalWaitingTime = totalWaitingTime + (cashConnect.tv_nsec - telDisconnect.tv_nsec);
        rc = pthread_mutex_unlock(&waitingTimeMutex);
        assert(rc == 0);


        rndSecCash = rndGen(T_CASH_LOW, T_CASH_HIGH);
        sleep(rndSecCash);

        currentCashInUse--;
        rc = pthread_cond_signal(&cashThresholdCond);
        assert(rc == 0);
        rc = pthread_mutex_unlock(&CashCounter);
        assert(rc == 0);

        choice = calcPCardSuccess();
        if (choice == 1)
        { // tha ginei h pliromh

            rc = pthread_mutex_lock(&addToBalance);
            assert(rc == 0);
            balance = balance + totalSeatsCost;

            successfulPayments++;
            clock_gettime(CLOCK_REALTIME, &custCompleted);
            printf("Customer %d successfull payment %d€\n", *threadId, totalSeatsCost);
            rc = pthread_mutex_unlock(&addToBalance);
            assert(rc == 0);
        }
        else if (choice == -1)
        { // den tha ginei h pliromi

            rc = pthread_mutex_lock(&PrintMutex);
            assert(rc == 0);
            printf("Customer %d: The reservation failed because the credit card transaction was not accepted\n", *threadId);
            rc = pthread_mutex_unlock(&PrintMutex);
            assert(rc == 0);

            // unsuccessfulPayments++;
            clock_gettime(CLOCK_REALTIME, &custCompleted);
            flagPayment = 0;
            creditCancellation++;
            pthread_exit(tId);
        }

        // totalsupporttime
        rc = pthread_mutex_lock(&supportTimeMutex);
        assert(rc == 0);

        totalSupportTime += custCompleted.tv_nsec - start.tv_nsec;
        rc = pthread_mutex_unlock(&supportTimeMutex);
        assert(rc == 0);

        // total waiting time
        rc = pthread_mutex_lock(&waitingTimeMutex);
        assert(rc == 0);

        rc = pthread_mutex_unlock(&waitingTimeMutex);
        assert(rc == 0);
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
    double avgWaitingTime,avgServingTime,avgSeatCancellation,avgCreditCancellation,avgSuccessTransact;
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
    assert(rc == 0);
    rc = pthread_mutex_init(&CashCounter, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&seatsMutex, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&PrintMutex, NULL); // den exw xrhsimopoihsei printmutex sthn
    assert(rc == 0);
    rc = pthread_mutex_init(&ticketFinder, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&addToBalance, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&waitingTimeMutex, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&supportTimeMutex, NULL);
    assert(rc == 0);

    rc = pthread_cond_init(&telThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&cashThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&ticketThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&balanceThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&printThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&timeThresholdCond, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&seatsThresholdCond, NULL);
    assert(rc == 0);

    pthread_t *threads = malloc(sizeof(pthread_t) * Ncust);
    if (threads == NULL)
    {
        printf("ERROR: Failed to allocate threads, not enough memory!\n");
        return -1;
    }
    printf("Main: Creating %d threads, one for each customer.\n", Ncust);
    int threadIds[Ncust];
    for (int i = 1; i <= Ncust; i++)
    {
        threadIds[i] = i;
        rc = pthread_create(&threads[i], NULL, customerServe, &threadIds[i]);
        assert(rc == 0);
        // wait for random seconds in [treslow, treshigh] before going to next thread creation
        int rndslpTime = rndGen(T_RES_LOW, T_RES_HIGH);
        sleep(rndslpTime);
    }

    // the end, wait for costumer threads to end
    void *status;
    for (int i = 1; i <= Ncust; i++)
    {
        rc = pthread_join(threads[i], &status);
        assert(rc == 0);
        printf("Main: Thread %d terminated successfully. \n",*(int *) status);
    }
    /*"katastrofi" mutex kai condition*/
    rc = pthread_mutex_destroy(&TelCounter);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&CashCounter);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&seatsMutex);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&PrintMutex);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&ticketFinder);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&addToBalance);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&waitingTimeMutex);
    assert(rc == 0);
    rc = pthread_mutex_destroy(&supportTimeMutex);
    assert(rc == 0);

    rc = pthread_cond_destroy(&telThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&cashThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&ticketThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&balanceThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&printThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&timeThresholdCond);
    assert(rc == 0);
    rc = pthread_cond_destroy(&seatsThresholdCond);
    assert(rc == 0);

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
    avgWaitingTime=totalWaitingTime /(double) Ncust;
    avgServingTime=totalSupportTime /(double) Ncust;
    avgSeatCancellation=(double) seatCancellation/ (double) Ncust;
    avgCreditCancellation=(double) creditCancellation/ (double) Ncust;
    avgSuccessTransact=(double) (Ncust - seatCancellation - creditCancellation) / (double) Ncust;
    printf("\nThe balance is: %d\n", balance);                      // Sunolika Esoda
    printf("Average waiting Time in nanoSec: %.2lf\n", avgWaitingTime); // Mesos xronos anamwnhs pelatwn
    printf("Average serving Time in nanoSec: %.2lf\n", avgServingTime);      // mesos xronos ejhpiretisis pelatwn
    printf("Percentage of lack of seats cancelled transactions : %.2lf%\n", avgSeatCancellation * 100);
    printf("Percentage of credit card failure cancelled transactions : %.2lf%\n", avgCreditCancellation * 100);
    printf("Percentage of successful transactions: %.2lf%\n", avgSuccessTransact * 100);

    return 1;
}
