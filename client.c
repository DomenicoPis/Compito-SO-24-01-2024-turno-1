#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "header_msg.h"
#include "header_sem.h"


int main() {

    srand(getpid());


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


    int num_valori = 10;

    msg_init_request req;
    msg_init_response res;


    printf("[CLIENT %d] Invio richiesta (num. valori: %d)\n", getpid(), num_valori);

    /* TBD: Inviare il messaggio di richiesta */

    req.mtype = 1;
    req.numero_valori = num_valori;
    req.pid_req = getpid();

    int err;

    err = msgsnd(id_coda_req, &req, sizeof(req) - sizeof(long), 0);
    if(err < 0){
        perror("errore msgsnd");
        exit(1);
    } 

    /* TBD: Ricevere il messaggio di risposta */

    err = msgrcv(id_coda_res, &res, sizeof(res) - sizeof(long), getpid(), 0);
    if(err < 0){
        perror("errore msgrcv");
        exit(1);
    }

    printf("[CLIENT %d] Ricevuto risposta\n", getpid());

    int id_shm_invio = res.id_shm_invio;
    int id_sem_invio = res.id_sem_invio;

    int id_shm_ricezione = res.id_shm_ricezione;
    int id_sem_ricezione = res.id_sem_ricezione;
    
    /* Faccio l'attach alle due memorie condivise */
    prodcons * p_invio = (prodcons *)shmat(id_shm_invio, NULL, 0);
    if (p_invio == (void *)-1) { 
        perror("Errore shmat invio"); 
        exit(1); 
    }

    prodcons * p_ricezione = (prodcons *)shmat(id_shm_ricezione, NULL, 0);
    if (p_ricezione == (void *)-1) { 
        perror("Errore shmat ricezione"); 
        exit(1); 
    }

    for(int i=0; i<num_valori; i++) {

        int valore = rand() % 10;


        produci(id_sem_invio, p_invio, valore);

        printf("[CLIENT %d] Valore inviato: %d\n", getpid(), valore);


        int ricevuto = consuma(id_sem_ricezione, p_ricezione);

        printf("[CLIENT %d] Valore ricevuto: %d\n", getpid(), ricevuto);

    }




}