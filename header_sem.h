#ifndef _HEADER_SEM_
#define _HEADER_SEM_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#define DIM 5

#define SPAZIO_DISP 0
#define MESSAGGIO_DISP 1
#define MUTEX 2

typedef struct prodcons {

    /* TBD: Definire la struttura con 
            un vettore circolare di buffer 
            di DIM=5 elementi, da condividere
            su shared memory UNIX
     */
    int buffer[DIM];
    int testa;
    int coda;
    int num_elementi;

} prodcons;


int Wait_Sem(int id_sem, int numsem);
int Signal_Sem(int id_sem, int numsem);

void produci(int id_sem, prodcons * p, int valore);
int consuma(int id_sem, prodcons * p);

#endif