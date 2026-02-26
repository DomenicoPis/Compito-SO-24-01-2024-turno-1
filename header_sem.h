#ifndef _HEADER_SEM_
#define _HEADER_SEM_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#define DIM 5

typedef struct prodcons {

    /* TBD: Definire la struttura con 
            un vettore circolare di buffer 
            di DIM=5 elementi, da condividere
            su shared memory UNIX
     */

    int buffer[DIM];
    int testa;
    int coda; 

} prodcons;

#define SPAZIO_DISPONIBILE 0
#define MESSAGGIO_DISPONIBILE 1
#define MUTEX_P 2
#define MUTEX_C 3


int Wait_Sem(int id_sem, int numsem);
int Signal_Sem(int id_sem, int numsem);

void produci(int id_sem, prodcons * p, int valore);
int consuma(int id_sem, prodcons * p);

#endif