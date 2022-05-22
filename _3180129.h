#pragma once


#ifndef PROJECT__3180129_H
#define PROJECT__3180129_H


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define SEATNUMTOTAL 300 				//total seats
#define SEATNUMZA   100        //number of seats for zone A and B
#define SEATNUMZB   200       // NzoneA * 10 = 10*10= 100, NzoneB * 10 = 20*10 = 200
#define NzoneA 10       //seires sth mprosta zoni
#define NzoneB 20       //seires sth pisw zoni
#define Nseat 10        //theseis ana seira
#define NTEL 3						//Telephonist
#define NCASH 2						//Cashier
#define PZONE_A 30					//Possibility of choosing zone A
#define N_SEAT_LOW 1				//random number of tickets
#define N_SEAT_HIGH 5
#define T_RES_LOW 1					//random sec of next customer calling
#define T_RES_HIGH 5
#define T_SEAT_LOW 5				//random sec of telephonist to check available seats
#define T_SEAT_HIGH 13
#define T_CASH_LOW 4				//random sec of cashier to check payment
#define T_CASH_HIGH 8
#define Pcardsuccess 90				//possibility of payment done
#define CzoneA 30				//cost of seats zone a
#define CzoneB 20				//cost of seats zone b
#define BILLION  1000000000L;


#endif //PROJECT__3180129_H
