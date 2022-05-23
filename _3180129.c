#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#define _XOPEN_SOURCE 500
#include "_3180129.h"

int Ncust;
unsigned int seed;
int *seatArrayA;
int *seatArrayB;
int pay_chance = 0;
// int zone_chance = 0;
int ticketcost = 0;
int balance = 0;
int id_transaction = 0;
double waitingTime = 0; // to be deleted
double startSupport = 0;
double totalWaitingTime = 0; // mesos xronos anamonhs
double totalSupportTime = 0; // mesos xronos eksipirethshs pelaton
int currentTelInUse = 0;
int currentCashInUse = 0;
double assistanceTime = 0;
int isProcessingSeats = 0;

pthread_mutex_t TelCounter;
pthread_mutex_t CashCounter;
pthread_mutex_t seatsAMutex;
pthread_mutex_t seatsBMutex;
pthread_mutex_t PrintMutex;

pthread_mutex_t ticketFinder;
pthread_mutex_t addToBalance;
pthread_mutex_t timeMutex;

pthread_cond_t telThresholdCond;
pthread_cond_t cashThresholdCond;
pthread_cond_t ticketThresholdCond;
pthread_cond_t balanceThresholdCond;
pthread_cond_t printThresholdCond;
pthread_cond_t timeThresholdCond;
pthread_cond_t seatsAThresholdCond;
pthread_cond_t seatsBThresholdCond;

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

/*
 *
// elegxos plirothtas kathe zonis
int isFullA(int tickets){
    int count = 0;
    for(int i = 0; i < SEATNUMZA; i++){
        if(seatArrayA[i] == 0){
            count = i;
            break;
        }
    }
    if((count + tickets) <= SEATNUMZA){
        return count;
    }
    else {
        return -1;
    }
    /*
     * int count = SEATNUMZA + 1;
    for(int i = 0; i < SEATNUMZA; i++){
        if(seatArrayA[i] == 0){
            count = i;
            break;
        }
    }

    if((count + tickets) <= SEATNUMZA){
        return count;
    }
    else {
        return -1;
    }
     */
/*
}

int isFullB(int tickets){

int count = SEATNUMZB + 1;
for(int i = 0; i < SEATNUMZB; i++){
 if(seatArrayB[i] == 0){
     count = i;
     break;
 }
}
if((count + tickets) <= SEATNUMZB){
 return count;
}
else {
 return -1;
}
}
*/

int reserveSeatsZA(int tickets, int tId)
{
    isProcessingSeats = 1;
    int seatsAvail = 0;
    // int custSeira = -1;
    // int custSeat = -1;
    for (int i = 0; i < SEATNUMZA; i++)
    { // an fullCheck >= 1 shmainei oti yparxei estw mia kenh thesi
        if (seatArrayA[i] == -1)
        {
            seatsAvail++;
        }
    }
    if (seatsAvail == 0)
    {
        printf("ERROR: The seats in requested zone are sold out, goodbye\n");
        return -1;
    }
    //  check contiguous values
    if (seatsAvail > tickets)
    {
        // iterate zone A
        for (int i = 0; i < NzoneA; i++)
        {
            // int count = 0;
            int helpCount = 0;
            int contigFlag = 0; // contiguous flag
            // gia kathe seira psakse an yparxoun diathesimes theseis
            for (int j = 0; j < Nseat; i++)
            { // theloume ksexwristh thn kathe seira,
                if (seatArrayA[i * 10 + j] == -1)
                { // gia ayto diplo for loop
                    for (int k = 1; k < tickets; k++)
                    { // an einai kenes kai oi epomenes tickets-1 theseis eimaste komple
                        if (seatArrayA[i * 10 + j + k] == -1)
                        {
                            helpCount++;
                        }
                    } // an yparxoun toses synexomenes theseis osa ta eisithria
                    if (helpCount >= tickets)
                    {
                        // custSeira = i;
                        contigFlag = 1;
                    }
                    if (contigFlag == 1)
                    { // an einai kenes kai oi epomenes tickets-1 theseis eimaste komple
                        for (int k = 0; k < tickets; k++)
                        {
                            seatArrayA[i * 10 + j + k] = tId;
                        } // oi theseis arithmountai apo to 0, den peirazei
                        printf("There are available seats sth seira %d, theseis %d ews %d in Zone A "
                               "for client %d, \n",
                               i, j, j + tickets);

                        return 1;
                    }
                    if (helpCount < tickets)
                    {
                        printf("There are available contiguous seats in Zone A \n");
                        return -2;
                    }
                }
            }
        }
    }
}

void *customerServe(void *tId)
{
    // struct contains two member variables:
    // tv_sec – The variable of the time_t type made to store time in seconds.
    // tv_nsec – The variable of the long type used to store time in nanoseconds.
    struct timespec start, stop;
    struct timespec custStopWait;
    struct timespec start2, stop2;
    int rndSeats;
    // int rndZone;
    int rndSecTel;
    int rndSecCash;
    int *threadId = (int *)tId;
    int rc;

    // begin counting support time for this thread
    clock_gettime(CLOCK_REALTIME, &start);
    /*
     *  MUTEX 0  //????????????
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from time_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }       //?????????????
     *
     */

    /* tilephonites eleftheroi mutex */
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
    printf("An available telephonist is found for customer $d\n", *threadId);
    currentTelInUse++;
    // begin counting time the customer is being serviced - talking on the phone
    clock_gettime(CLOCK_REALTIME, &custStopWait);
    // add this customer's waiting time until telephonist support, to total
    totalWaitingTime = totalWaitingTime + (custStopWait.tv_nsec - start.tv_nsec);

    rc = pthread_mutex_unlock(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }
    /*
     // MUTEX 1.2
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    //waitingTime += ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    // MUTEX 1.3 //
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &start2) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }
     */

    // calculate probability of zone selection
    // alternate if: (not so good)
    //  if( (double)rand()/3.0 > (double)RAND_MAX/10.0 ){ //select zone A
    if (calcPzoneA() == 1)
    { // select zone A
        // select random number of tickets the client wants
        rndSeats = rndGen(N_SEAT_LOW, N_SEAT_HIGH);
        // time the telephonist needs to determine availability of continuous seats in selected zone
        int rndSeatAvailTime = rndGen(T_SEAT_LOW, T_SEAT_HIGH);
        sleep(rndSeatAvailTime); // thread goes sleeping for this time

        rc = pthread_mutex_lock(&seatsAMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock(&seatsAMutex) is %d\n", rc);
            pthread_exit(&rc);
        }
        while (isProcessingSeats == 1)
        {
            printf("Another thread is processing the theater, please wait, (from thread %d)\n", *threadId);
            rc = pthread_cond_wait(&seatsAThresholdCond, &seatsAMutex);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                pthread_exit(&rc);
            }

            printf("increaseCount(): thread %d, the thread that will double the counter has started.\n",
                   *threadId);
        }
        if (reserveSeatsZA(rndSeats, *threadId) == -1)
        {
            printf("The seats in requested zone are sold out, goodbye\n");
        }
        else if (reserveSeatsZA(rndSeats, *threadId) == -2)
        {
            printf("The number of tickets you asked (%d) is not available in zone A, goodbye\n", rndSeats);
        }
        else if (reserveSeatsZA(rndSeats, *threadId) == 1)
        { // there are enough available seats
            rc = pthread_mutex_unlock(&seatsAMutex);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_mutex_unlock(&seatsAMutex) is %d\n", rc);
                pthread_exit(&rc);
            }
            isProcessingSeats = 0;
            rc = pthread_cond_broadcast(&seatsAThresholdCond);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_cond_broadcast(&seatsAThresholdCond) is %d\n", rc);
                pthread_exit(&rc);
            }
        }
    }
    else
    { // select zone B
    }

    /*
     * //rndSeats= rndGen(N_SEAT_LOW,N_SEAT_HIGH);
    //rndZone= rndGen(); gia na vroume se poia zoni kai na orisoyme kai to sunoliko kostos
     * //zone_chance = rand_r(&seed)% 100;
    //if (zone_chance >=30){
    //		ticketcost =20
    //} else {
    //		ticketcost = 30
    //}
    //rndSecTel= rndGen(T_SEAT_LOW,T_SEAT_HIGH);
     */

    //* MUTEX 2 */
    rc = pthread_mutex_lock(&CashCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    while (currentCashInUse >= NCASH)
    {
        printf("Currently waiting for the first available Cashier, please wait, %d ---\n", currentCashInUse);

        rc = pthread_cond_wait(&thresholdCond, &CashCounter);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }
        printf("An available Cashier is found for customer\n");
    }

    currentCashInUse++;

    rc = pthread_mutex_unlock(&CashCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 2.2 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1)
    {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    waitingTime += (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 2.3 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (clock_gettime(CLOCK_REALTIME, &start2) == -1)
    {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rndSecCash = rndGen(T_CASH_LOW, T_CASH_HIGH);

    /* MUTEX 3 */
    rc = pthread_mutex_lock(&ticketFinder);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    sleep(rndSecTel);
    id_transaction++;
    int count = _isFull(rndSeats);
    if (count != -1)
    {

        srand(seed);
        pay_chance = rand() % 100;
        printf("This is the chance you get.. %d %%\n", pay_chance);
        if (pay_chance = 10)
        { //!!!!!
            printf("Your payment failed! Sorry for the inconvenience..");

            /* MUTEX 5/1 */
            rc = pthread_mutex_lock(&timeMutex);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                pthread_exit(&rc);
            }

            if (clock_gettime(CLOCK_REALTIME, &stop2) == -1)
            {
                perror("clock gettime");
                exit(EXIT_FAILURE);
            }

            waitingTime += rndSecCash + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;
            assistanceTime += waitingTime + rndSecCash + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;

            rc = pthread_mutex_unlock(&timeMutex);
            if (rc != 0)
            {
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }

            rc = pthread_mutex_unlock(&ticketFinder);
            pthread_exit(&rc);
        }

        for (int i = count; i < count + rndSeats; i++)
        {
            seatArray[i] = id_transaction;
        }
    }
    else if (count == -1)
    {
        printf("All seats are reserved, thank you %d customer!:)\n", id_transaction);

        /* MUTEX 6.1 */
        rc = pthread_mutex_lock(&timeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        if (clock_gettime(CLOCK_REALTIME, &stop2) == -1)
        {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        sleep(rndSecCash);
        assistanceTime += waitingTime + rndSecTel + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;

        rc = pthread_mutex_unlock(&timeMutex);
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }

        rc = pthread_mutex_unlock(&ticketFinder);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_unlock(&ticketFinder);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 4 */
    rc = pthread_mutex_lock(&addToBalance);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    balance += rndSeats * ticketcost;

    rc = pthread_mutex_unlock(&addToBalance);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 5 */
    rc = pthread_mutex_lock(&PrintMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }
    if (count != -1)
    {

        printf("Your Transaction id is :%d\n", id_transaction);
        printf("Your seats are succesfully reserved. We shall now process to payment\n");
        printf("Total cost of the transaction is: %d€\n", rndSeats * ticketcost);
        printf("Your Seats reserved are: ");
        for (int i = count; i < count + rndSeats; i++)
        {
            printf(" %d ", i);
        }
        printf("\n");
    }

    rc = pthread_mutex_unlock(&PrintMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 6 */
    rc = pthread_mutex_lock(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_cond_signal(&thresholdCond);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
        pthread_exit(&rc);
    }

    currentTelInUse--;

    rc = pthread_mutex_unlock(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 5/1 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (clock_gettime(CLOCK_REALTIME, &stop2) == -1)
    {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    assistanceTime += waitingTime + rndSecTel + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    pthread_exit(threadId);

    /* MUTEX 7 */
    rc = pthread_mutex_lock(&CashCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_cond_signal(&thresholdCond);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
        pthread_exit(&rc);
    }

    currentCashInUse--;

    rc = pthread_mutex_unlock(&CashCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 5/1 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (clock_gettime(CLOCK_REALTIME, &stop2) == -1)
    {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    assistanceTime += waitingTime + rndSecCash + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    pthread_exit(threadId);
}

int main(int argc, char *argv[])
{

    int rc;
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

    // seats for zone A
    seatArrayA = (int *)malloc(sizeof(int) * SEATNUMZA);
    if (seatArrayA == NULL)
    {
        printf("ERROR: Malloc (seatArrayA) failed not enough memory!\n");
        return -1;
    }
    // Array initialization , all elements should be 0 means all seats are free, for now
    for (int i = 0; i < SEATNUMZA; i++)
    {
        seatArrayA[i] = 0;
    }
    //  seats for zone B
    seatArrayB = (int *)malloc(sizeof(int) * SEATNUMZB);
    if (seatArrayB == NULL)
    {
        printf("ERROR: Malloc (seatArrayB) failed not enough memory!\n");
        return -1;
    }
    for (int i = 0; i < SEATNUMZB; i++)
    {
        seatArrayB[i] = 0;
    }

    rc = pthread_mutex_init(&TelCounter, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_init() (TelCounter) is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&addToBalance, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_init() (addToBalance) is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&ticketFinder, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_init() (ticketFinder) is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&PrintMutex, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_init() (PrintMutex) is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&timeMutex, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_init() (timeMutex) is %d\n", rc);
        exit(-1);
    }
    rc = pthread_cond_init(&thresholdCond, NULL);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_init() (thresholdCond) is %d\n", rc);
        exit(-1);
    }

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
        if (rc != 0)
        {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    // the end, wait for costumer threads to end
    void *status;
    for (int i = 0; i < Ncust; i++)
    {
        rc = pthread_join(threads[i], &status);

        if (rc != 0)
        {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    // ?????
    for (int i = 0; i < SEATNUMZA; i++)
    {
        printf("seatArrayA[%d] = %d.\n", i, *(seatArrayA + i));
    }
    for (int i = 0; i < SEATNUMZB; i++)
    {
        printf("seatArrayB[%d] = %d.\n", i, *(seatArrayB + i));
    }

    printf("The balance is: %d\n", balance);                   // Plano thesewn
    printf("Total transactions: %d\n", id_transaction);        // Sunolika Esoda
    printf("Average waiting Time: %f\n", waitingTime / Ncust); // Mesos xronos anamwnhs pelatwn
    printf("Average serving: %f\n", assistanceTime / Ncust);   // mesos xronos ejhpiretisis pelatwn

    rc = pthread_mutex_destroy(&TelCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&CashCounter);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&addToBalance);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&ticketFinder);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&PrintMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&timeMutex);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_cond_destroy(&thresholdCond);
    if (rc != 0)
    {
        printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
        exit(-1);
    }

    free(threads);
    free(seatArrayA);
    free(seatArrayB);

    return 1;
}
