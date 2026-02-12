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


void server(/* TBD: Completare il passaggio di parametri */);


int main() {


    /* TBD: Ottenere gli identificativi delle code di messaggi */

    int id_coda_req = msgget(ftok(".", 'a'), 0);
    int id_coda_res = msgget(ftok(".", 'b'), 0);

    if (id_coda_req < 0 || id_coda_res < 0) {

        perror("Errore msgget");

        exit(1);

    }

    for(int richieste=0; richieste<3; richieste++) {

        msg_init_request req;
        msg_init_response res;


        /* TBD: ricevere un messaggio dal client */

        msgrcv(id_coda_req, &req, sizeof(msg_init_request) - sizeof(long), 0, 0);

        int num_valori = req.numero_valori;
        int pid_client = req.pid_richiesta;

        printf("[SERVER %d] Ricezione richiesta (num. valori: %d)\n", getpid(), num_valori);


        /* TBD: Allocare e inizializzare le risorse che saranno condivise
                tra il client e il processo server figlio */

        int id_sem_invio = semget(IPC_PRIVATE, 3, IPC_CREAT | 0664);
        int id_shm_invio = shmget(IPC_PRIVATE, sizeof(prodcons), IPC_CREAT | 0664);
        int id_sem_ricezione = semget(IPC_PRIVATE, 3, IPC_CREAT | 0664);
        int id_shm_ricezione = shmget(IPC_PRIVATE, sizeof(prodcons), IPC_CREAT | 0664);

        if(id_shm_invio < 0 || id_sem_invio < 0 || id_shm_ricezione < 0 || id_sem_ricezione < 0) {
            perror("Errore allocazione risorse private");
            exit(1);
        }

        // Inizializzazione Semafori INVIO (Client -> Server)
        semctl(id_sem_invio, SPAZIO_DISP, SETVAL, DIM); // Buffer vuoto = 5 spazi liberi
        semctl(id_sem_invio, MESSAGGIO_DISP, SETVAL, 0); // 0 messaggi da leggere
        semctl(id_sem_invio, MUTEX, SETVAL, 1);         // Mutex verde

        // Inizializzazione Semafori RICEZIONE (Server -> Client)
        semctl(id_sem_ricezione, SPAZIO_DISP, SETVAL, DIM);
        semctl(id_sem_ricezione, MESSAGGIO_DISP, SETVAL, 0);
        semctl(id_sem_ricezione, MUTEX, SETVAL, 1);

        // Inizializzazione Memoria Condivisa (Testa e Coda a 0)
        prodcons *p_temp = (prodcons *)shmat(id_shm_invio, NULL, 0);
        p_temp->testa = 0; p_temp->coda = 0;
        shmdt(p_temp);

        p_temp = (prodcons *)shmat(id_shm_ricezione, NULL, 0);
        p_temp->testa = 0; p_temp->coda = 0;
        shmdt(p_temp);

        /* TBD: Inviare il messaggio di risposta al client, 
                con gli identificativi delle risorse condivise
         */

        res.mtype = pid_client; // IMPORTANTE: Indirizzo la risposta specificamente al PID del client
        res.id_shm_invio = id_shm_invio;
        res.id_sem_invio = id_sem_invio;
        res.id_shm_ricezione = id_shm_ricezione;
        res.id_sem_ricezione = id_sem_ricezione;

        if (msgsnd(id_coda_res, &res, sizeof(msg_init_response) - sizeof(long), 0) < 0) {
            perror("Errore msgsnd risposta");
            exit(1);
        }

        printf("[SERVER %d] Inviato risposta\n", getpid());

    

        /* TBD: Creare un processo server figlio, e
                passargli i riferimenti alle risorse condivise
         */

        pid_t pid = fork();
        if (pid == 0) {
            // Processo Figlio
            server(&req, &res);
            exit(0); // Il figlio termina dopo aver servito il client
        }
        else if (pid < 0) {
            perror("Errore fork");
            exit(1);
        }

    }

}



void server(msg_init_request * req, msg_init_response * res) {

    int num_valori = req->num_valori;

    prodcons * p_invio = (prodcons *) shmat(res->id_shm_invio, NULL, 0);
    prodcons * p_ricezione = (prodcons *) shmat(res->id_shm_ricezione, NULL, 0);

    if (p_invio == (void*)-1 || p_ricezione == (void*)-1) {
        perror("Errore shmat figlio");
        exit(1);
    }

    for(int i=0; i<num_valori; i++) {

        int ricevuto = consuma(res->id_sem_invio, p_invio);

        printf("[SERVER %d] Valore ricevuto: %d\n", getpid(), ricevuto);

        int valore = ricevuto * 2;

        produci(res->id_sem_ricezione, p_ricezione, valore);

        printf("[SERVER %d] Valore inviato: %d\n", getpid(), valore);

    }

    shmdt(p_invio);
    shmdt(p_ricezione);

    // Rimuovo le risorse PRIVATE create dal padre per questo client
    shmctl(res->id_shm_invio, IPC_RMID, NULL);
    semctl(res->id_sem_invio, 0, IPC_RMID);
    
    shmctl(res->id_shm_ricezione, IPC_RMID, NULL);
    semctl(res->id_sem_ricezione, 0, IPC_RMID);

}