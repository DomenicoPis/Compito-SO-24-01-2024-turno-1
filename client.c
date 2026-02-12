#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "header_msg.h"
#include "header_sem.h"

int main() {

    srand(getpid() * time(NULL)); // Inizializzo il seed per numeri casuali

    // --- 1. CONNESSIONE ALLE CODE PUBBLICHE ---
    // Uso le stesse chiavi di start.c e server.c ('a' per richieste, 'b' per risposte)
    int id_coda_req = msgget(ftok(".", 'a'), 0);
    int id_coda_res = msgget(ftok(".", 'b'), 0);

    if(id_coda_req < 0 || id_coda_res < 0){
        perror("Errore msgget client (start non avviato?)");
        exit(1);
    }

    // --- 2. PREPARAZIONE DELLA RICHIESTA ---
    msg_init_request req;
    
    // CAMPI CORRETTI COME DA TUA INDICAZIONE:
    req.mtype = 1;                 // Tipo > 0 obbligatorio
    req.pid_richiesta = getpid();  // Il mio PID (fondamentale per ricevere la risposta)
    req.numero_valore = 10;        // Quanti valori voglio scambiare

    printf("[CLIENT %d] Invio richiesta (valori: %d)\n", getpid(), req.numero_valore);

    // Invio la richiesta sulla coda 'a'
    if(msgsnd(id_coda_req, &req, sizeof(msg_init_request) - sizeof(long), 0) < 0) {
        perror("Errore msgsnd richiesta");
        exit(1);
    }

    // --- 3. RICEZIONE DELLA RISPOSTA ---
    msg_init_response res;

    // Attendo un messaggio che abbia come tipo ESATTAMENTE il mio PID
    // msgrcv(id, &struct, size, TIPO, flag)
    if(msgrcv(id_coda_res, &res, sizeof(msg_init_response) - sizeof(long), getpid(), 0) < 0) {
        perror("Errore msgrcv risposta");
        exit(1);
    }

    printf("[CLIENT %d] Ricevute risorse dal server. Inizio lavoro.\n", getpid());


    // --- 4. ATTACH ALLA MEMORIA (USO GLI ID RICEVUTI) ---
    // Non uso IPC_PRIVATE, uso gli ID che mi ha mandato il server
    
    prodcons * p_invio = (prodcons *) shmat(res.id_shm_invio, NULL, 0);
    prodcons * p_ricezione = (prodcons *) shmat(res.id_shm_ricezione, NULL, 0);

    if (p_invio == (void*)-1 || p_ricezione == (void*)-1) {
        perror("Errore shmat client");
        exit(1);
    }


    // --- 5. CICLO DI LAVORO ---
    for(int i=0; i < req.numero_valore; i++) {

        int valore = rand() % 10;

        // INVIO AL SERVER (Produco nel buffer di invio)
        produci(res.id_sem_invio, p_invio, valore);
        printf("[CLIENT %d] Inviato: %d\n", getpid(), valore);

        // RICEVO DAL SERVER (Consumo dal buffer di ricezione)
        int ricevuto = consuma(res.id_sem_ricezione, p_ricezione);
        printf("[CLIENT %d] Ricevuto: %d\n", getpid(), ricevuto);
    }

    // --- 6. DISTACCO (DETACH) ---
    shmdt(p_invio);
    shmdt(p_ricezione);

    printf("[CLIENT %d] Terminato.\n", getpid());
    return 0;
}
