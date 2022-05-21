
#pragma once


#ifndef PROJECT__3180129_H
#define PROJECT__3180129_H


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define SEATNUM 3000 				//total seats
#define LINES_ZONE_A 10
#define LINES_ZONE_B 20
#define SEATS_PER_LINE 10
#define TEL 3						//Telephonist
#define CASH 2						//Cashier
#define PZONE_A 30					//Possibility of choosing zone A
#define N_SEAT_LOW 1				//random number of tickets
#define N_SEAT_HIGH 5
#define T_RES_LOW 1					//random sec of next customer calling
#define T_RES_HIGH 5
#define T_SEAT_LOW 5				//random sec of telephonist to check available seats
#define T_SEAT_HIGH 13
#define T_CASH_LOW 4				//random sec of cashier to check payment
#define T_CASH_HIGH 8
#define P_CARDSUCCESS 90				//possibility of payment done
#define SEATCOST_A 30				//cost of seats zone a
#define SEATCOST_B 20				//cost of seats zone b
#define BILLION  1000000000L;


#endif //PROJECT__3180129_H
