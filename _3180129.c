#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "cmake-build-debug/_3180129.h"

int Ncust;
unsigned int seed;
int *seatArrayA;
int *seatArrayB;
int pay_chance = 0;
//int zone_chance = 0;
int ticketcost = 0 ;
int balance = 0;
int id_transaction = 0;
double waitingTime = 0;
int currentTelInUse = 0;
int currentCashInUse = 0;
double assistanceTime = 0;



pthread_mutex_t TelCounter;
pthread_mutex_t CashCounter;
pthread_mutex_t ticketFinder;
pthread_mutex_t addToBalance;
pthread_mutex_t PrintMutex;
pthread_mutex_t timeMutex;

pthread_cond_t thresholdCond;


int rndGen(int low,int high)
{
    int result ;    //first seed the random number generator
    srand(seed);
    result= ( (rand() % (high+1-low) ) + low );
    return result;
}

// elegxos plirothtas kathe zonis
int _isFullA(int tickets){

    int count = SEATNUMZA + 1;
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
}

int _isFullB(int tickets){

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

void *customerServe(void *threadId) {

    struct timespec start, stop;
    struct timespec start2, stop2;
    int rndSeats;
    //int rndZone;
    int rndSecTel;
    int rndSecCash;
    int rc;

    /* MUTEX 0 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
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
    }


    /* MUTEX 1 */
    rc = pthread_mutex_lock(&TelCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }


    while(currentTelInUse>=TEL){
        printf("Currently waiting for the first available Telephonist, please wait, %d ---\n", currentTelInUse);

        rc= pthread_cond_wait(&thresholdCond ,&TelCounter);
        if(rc!=0){
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }
        printf("An available telephonist is found for customer\n");

    }

    currentTelInUse++;

    rc = pthread_mutex_unlock(&TelCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    /* MUTEX 1.2 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    waitingTime += ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 1.3 */
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

    rndSeats= rndGen(N_SEAT_LOW,N_SEAT_HIGH);
    //rndZone= rndGen(); gia na vroume se poia zoni kai na orisoyme kai to sunoliko kostos
    //zone_chance = rand_r(&seed)% 100;
    //if (zone_chance >=30){
    //		ticketcost =20
    //} else {
    //		ticketcost = 30
    //}
    //
    rndSecTel= rndGen(T_SEAT_LOW,T_SEAT_HIGH);

    //* MUTEX 2 */
    rc = pthread_mutex_lock(&CashCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }


    while(currentCashInUse>=CASH){
        printf("Currently waiting for the first available Cashier, please wait, %d ---\n", currentCashInUse);

        rc= pthread_cond_wait(&thresholdCond ,&CashCounter);
        if(rc!=0){
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }
        printf("An available Cashier is found for customer\n");

    }

    currentCashInUse++;

    rc = pthread_mutex_unlock(&CashCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    /* MUTEX 2.2 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    waitingTime += ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 2.3 */
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

    rndSecCash= rndGen(T_CASH_LOW,T_CASH_HIGH);


    /* MUTEX 3 */
    rc = pthread_mutex_lock(&ticketFinder);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    sleep(rndSecTel);
    id_transaction++;
    int count = _isFull(rndSeats);
    if(count != -1){

        srand(seed);
        pay_chance = rand()% 100;
        printf("This is the chance you get.. %d %%\n", pay_chance);
        if (pay_chance = 10){ //!!!!!
            printf("Your payment failed! Sorry for the inconvenience..");


            /* MUTEX 5/1 */
            rc = pthread_mutex_lock(&timeMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                pthread_exit(&rc);
            }

            if( clock_gettime( CLOCK_REALTIME, &stop2) == -1 ) {
                perror( "clock gettime" );
                exit( EXIT_FAILURE );
            }

            waitingTime += rndSecCash + ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;
            assistanceTime += waitingTime + rndSecCash + ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

            rc = pthread_mutex_unlock(&timeMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }


            rc = pthread_mutex_unlock(&ticketFinder);
            pthread_exit(&rc);

        }

        for(int i = count; i< count + rndSeats; i++){
            seatArray[i] = id_transaction;
        }



    } else if(count == -1){
        printf("All seats are reserved, thank you %d customer!:)\n", id_transaction);

        /* MUTEX 6.1 */
        rc = pthread_mutex_lock(&timeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        if( clock_gettime( CLOCK_REALTIME, &stop2) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }

        sleep(rndSecCash);
        assistanceTime += waitingTime + rndSecTel + ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

        rc = pthread_mutex_unlock(&timeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }

        rc = pthread_mutex_unlock(&ticketFinder);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_unlock(&ticketFinder);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 4 */
    rc = pthread_mutex_lock(&addToBalance);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    balance+=rndSeats*ticketcost;

    rc = pthread_mutex_unlock(&addToBalance);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    /* MUTEX 5 */
    rc = pthread_mutex_lock(&PrintMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }
    if( count!=-1 ){

        printf("Your Transaction id is :%d\n", id_transaction);
        printf("Your seats are succesfully reserved. We shall now process to payment\n");
        printf("Total cost of the transaction is: %d€\n", rndSeats*ticketcost);
        printf("Your Seats reserved are: ");
        for(int i = count; i< count + rndSeats; i++){
            printf(" %d ", i );
        }
        printf("\n");

    }

    rc = pthread_mutex_unlock(&PrintMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    /* MUTEX 6 */
    rc = pthread_mutex_lock(&TelCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_cond_signal(&thresholdCond);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
        pthread_exit(&rc);
    }

    currentTelInUse--;


    rc = pthread_mutex_unlock(&TelCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 5/1 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &stop2) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    assistanceTime += waitingTime + rndSecTel + ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    pthread_exit(threadId);

    /* MUTEX 7 */
    rc = pthread_mutex_lock(&CashCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_cond_signal(&thresholdCond);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
        pthread_exit(&rc);
    }

    currentCashInUse--;


    rc = pthread_mutex_unlock(&CashCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    /* MUTEX 5/1 */
    rc = pthread_mutex_lock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if( clock_gettime( CLOCK_REALTIME, &stop2) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    assistanceTime += waitingTime + rndSecCash + ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

    rc = pthread_mutex_unlock(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }


    pthread_exit(threadId);
}



int main(int argc, char *argv[]) {

    int rc;
    //Checks if user gave the correct input
    if (argc != 3) {
        printf("ERROR: the program should take two arguments, the number of customers to create and the seed number!\n");
        exit(0);
    }
    // convert input char* to int
    Ncust = atoi(argv[1]);
    seed = atoi(argv[2]);

    //Check if the input values are positive numbers, otherwise end program
    if (seed < 0) {
        printf("ERROR: the number of seed should be a positive number. Current number given %d.\n", seed);
        exit(-1);
    }
    if (Ncust <= 0 ) {
        printf("ERROR: the number of customers should be a positive number. Current number given %d.\n", Ncust);
        exit(-1);
    }

    // seats for zone A
    seatArrayA = (int *)malloc(sizeof(int) * SEATNUMZA);
    //elegxos an apetyxe i malloc alla mallon prepei na ginei ana zoni pali
    if (seatArrayA == NULL) {
        printf("ERROR: Malloc (seatArrayA) failed not enough memory!\n");
        return -1;
    }
    //Array initialization , all elements should be 0 means all seats are free, for now
    for(int i = 0; i < SEATNUMZA; i++) {
        seatArrayA[i]=0;
    }
    //  seats for zone B
    seatArrayB = (int *)malloc(sizeof(int) * SEATNUMZB);
    if (seatArrayB == NULL) {
        printf("ERROR: Malloc (seatArrayB) failed not enough memory!\n");
        return -1;
    }
    for(int i = 0; i < SEATNUMZB; i++) {
        seatArrayB[i]=0;
    }

    rc = pthread_mutex_init(&TelCounter, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&addToBalance, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&ticketFinder, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&PrintMutex, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&timeMutex, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc= pthread_cond_init(&thresholdCond, NULL);
    if (rc!=0){
        printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
        exit(-1);
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * Ncust);
    if (threads == NULL) {
        printf("ERROR: Failed to allocate threads, not enough memory!\n");
        return -1;
    }
    printf("Main: Creating %d threads, one for each customer.\n", Ncust);
    int threadIds[Ncust];
    for (int i = 0; i < Ncust; i++) {
        threadIds[i] = i;
        //MARIA MOU edw sto allaksa evala i anti gia i+1, IDKW
        rc = pthread_create(&threads[i], NULL, customerServe, &threadIds[i]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    //the end, wait for costumer threads to end
    void *status;
    for (int i = 0; i < Ncust; i++) {
        rc = pthread_join(threads[i], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

    }

    // ?????
    for (int i = 0; i < SEATNUMZA; i++) {
        printf("seatArrayA[%d] = %d.\n", i, *(seatArrayA + i));
    }
    for (int i = 0; i < SEATNUMZB; i++) {
        printf("seatArrayB[%d] = %d.\n", i, *(seatArrayB + i));
    }

    printf("The balance is: %d\n",balance);										//Plano thesewn
    printf("Total transactions: %d\n",id_transaction);							//Sunolika Esoda
    printf("Average waiting Time: %f\n",waitingTime/Ncust);					//Mesos xronos anamwnhs pelatwn
    printf("Average serving: %f\n",assistanceTime/Ncust);					//mesos xronos ejhpiretisis pelatwn


    rc = pthread_mutex_destroy(&TelCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&CashCounter);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&addToBalance);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&ticketFinder);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&PrintMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&timeMutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_cond_destroy(&thresholdCond);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
        exit(-1);
    }


    free(threads);
    free(seatArrayA);
    free(seatArrayB);


    return 1;
}
