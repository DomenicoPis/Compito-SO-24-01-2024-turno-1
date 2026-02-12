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

    srand(getpid()); // Inizializzo generatore numeri casuali

    // --- 1. CONNESSIONE ALLE CODE PUBBLICHE ---
    // Uso le stesse chiavi di start.c e server.c ('a' e 'b')
    int id_coda_req = msgget(ftok(".", 'a'), 0);
    int id_coda_res = msgget(ftok(".", 'b'), 0);

    if(id_coda_req < 0 || id_coda_res < 0){
        perror("Errore msgget client");
        exit(1);
    }

    int num_valori = 10;
    msg_init_request req;
    msg_init_response res;

    // --- 2. PREPARAZIONE E INVIO RICHIESTA ---
    req.mtype = 1;              // Tipo messaggio (arbitrario, basta che sia > 0)
    req.pid_client = getpid();  // Il mio PID (fondamentale per la risposta)
    req.num_valori = num_valori;

    printf("[CLIENT %d] Invio richiesta (num. valori: %d)\n", getpid(), num_valori);

    // Invio la richiesta
    if(msgsnd(id_coda_req, &req, sizeof(msg_init_request) - sizeof(long), 0) < 0) {
        perror("Errore msgsnd");
        exit(1);
    }

    // --- 3. RICEZIONE RISPOSTA ---
    // Attendo un messaggio che abbia come tipo il MIO PID (getpid())
    // Questo è fondamentale per non leggere le risposte destinate ad altri client!
    if(msgrcv(id_coda_res, &res, sizeof(msg_init_response) - sizeof(long), getpid(), 0) < 0) {
        perror("Errore msgrcv");
        exit(1);
    }

    printf("[CLIENT %d] Ricevute risorse dal server. Inizio lavoro.\n", getpid());


    // --- 4. ATTACH ALLA MEMORIA (USO GLI ID RICEVUTI) ---
    // NON creo nulla, uso gli ID che mi ha mandato il server nella struct 'res'
    
    prodcons * p_invio = (prodcons *) shmat(res.id_shm_invio, NULL, 0);
    prodcons * p_ricezione = (prodcons *) shmat(res.id_shm_ricezione, NULL, 0);

    if (p_invio == (void*)-1 || p_ricezione == (void*)-1) {
        perror("Errore shmat client");
        exit(1);
    }


    // --- 5. CICLO DI LAVORO ---
    for(int i=0; i<num_valori; i++) {

        int valore = rand() % 10;

        // INVIO AL SERVER
        // Uso l'ID del semaforo ricevuto e il puntatore alla memoria collegata
        produci(res.id_sem_invio, p_invio, valore);
        printf("[CLIENT %d] Valore inviato: %d\n", getpid(), valore);

        // RICEVO DAL SERVER
        int ricevuto = consuma(res.id_sem_ricezione, p_ricezione);
        printf("[CLIENT %d] Valore ricevuto: %d\n", getpid(), ricevuto);
    }

    // --- 6. DISTACCO (DETACH) ---
    // Il client si stacca solo dalla memoria. 
    // La rimozione (IPC_RMID) la fa il server figlio quando ha finito.
    shmdt(p_invio);
    shmdt(p_ricezione);

    printf("[CLIENT %d] Terminato.\n", getpid());
    return 0;
}