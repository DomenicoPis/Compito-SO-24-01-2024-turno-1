#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "header_msg.h"
#include "header_sem.h"


void server(prodcons * p_invio, prodcons * p_ricezione, int id_sem_invio, int id_sem_ricezione, int iterazioni);


int main() {


    /* TBD: Ottenere gli identificativi delle code di messaggi */

    int chiave_req = ftok(".", 'a');
    int id_coda_req = msgget(chiave_req, 0);
    if(id_coda_req < 0){
        perror("errore msgget invio");
        exit(1);
    }

    int chiave_res = ftok(".", 'b');
    int id_coda_res = msgget(chiave_res, 0);
    if(id_coda_res < 0){
        perror("errore msgget risposta");
        exit(1);
    }

    for(int richieste=0; richieste<3; richieste++) {

        msg_init_request req;
        msg_init_response res;

        int err;

        /* TBD: ricevere un messaggio dal client */

    err = msgrcv(id_coda_req, &req, sizeof(msg_init_response) - sizeof(long), 1, 0);        if(err < 0){
            perror("errore msgrcv");
            exit(1);
        }

        int num_valori = req.numero_valori;
        int pid_client = req.pid_req;

        printf("[SERVER %d] Ricezione richiesta (num. valori: %d)\n", getpid(), num_valori);


        /* TBD: Allocare e inizializzare le risorse che saranno condivise
                tra il client e il processo server figlio
         */

    int id_shm_invio = shmget(IPC_PRIVATE, sizeof(prodcons), IPC_CREAT | 0664);
        prodcons * p_invio = (prodcons *)shmat(id_shm_invio, NULL, 0);
        p_invio->testa = 0;
        p_invio->coda = 0;

        int id_sem_invio = semget(IPC_PRIVATE, 2, IPC_CREAT | 0664);
        semctl(id_sem_invio, SPAZIO_DISPONIBILE, SETVAL, DIM);
        semctl(id_sem_invio, MESSAGGIO_DISPONIBILE, SETVAL, 0);

        // 2. Vettore di RICEZIONE (Server -> Client)
        int id_shm_ricezione = shmget(IPC_PRIVATE, sizeof(prodcons), IPC_CREAT | 0664);
        prodcons * p_ricezione = (prodcons *)shmat(id_shm_ricezione, NULL, 0);
        p_ricezione->testa = 0;
        p_ricezione->coda = 0;

        int id_sem_ricezione = semget(IPC_PRIVATE, 2, IPC_CREAT | 0664);
        semctl(id_sem_ricezione, SPAZIO_DISPONIBILE, SETVAL, DIM);
        semctl(id_sem_ricezione, MESSAGGIO_DISPONIBILE, SETVAL, 0);

        /* Inviare il messaggio di risposta al client */
        res.mtype = pid_client; // Indirizzo il messaggio a QUESTO client specifico
        res.id_shm_invio = id_shm_invio;
        res.id_sem_invio = id_sem_invio;
        res.id_shm_ricezione = id_shm_ricezione;
        res.id_sem_ricezione = id_sem_ricezione;
         
        err = msgsnd(id_coda_res, &res, sizeof(msg_init_response) - sizeof(long), 0);
        if(err < 0){
            perror("errore msgsnd");
            exit(1);
        }

        printf("[SERVER %d] Inviato risposta\n", getpid());

    

        /* TBD: Creare un processo server figlio, e
                passargli i riferimenti alle risorse condivise
         */

        pid_t pid = fork();

        if(pid == 0){
            server(p_invio, p_ricezione, id_sem_invio, id_sem_ricezione, num_valori);
            exit(0);
            exit(0);
        }else if(pid < 0){
            perror("errore fork");
            exit(1);
        }


    }

}



void server(prodcons * p_invio, prodcons * p_ricezione, int id_sem_invio, int id_sem_ricezione, int iterazioni) {

    int num_valori = iterazioni;

    for(int i=0; i<num_valori; i++) {

        int ricevuto = consuma(id_sem_invio, p_invio);

        printf("[SERVER %d] Valore ricevuto: %d\n", getpid(), ricevuto);

        int valore = ricevuto * 2;

        produci(id_sem_ricezione, p_ricezione, valore);

        printf("[SERVER %d] Valore inviato: %d\n", getpid(), valore);

    }

}