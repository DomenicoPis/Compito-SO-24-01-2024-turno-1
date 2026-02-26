#ifndef _HEADER_MSG_
#define _HEADER_MSG_

typedef struct {

    /* TBD: Definire il messaggio di richiesta */
    long mtype;
    int pid_req;
    int numero_valori;

} msg_init_request;

typedef struct {

    /* TBD: Definire il messaggio di risposta */
    long mtype;
    int id_sem_invio;
    int id_shm_invio;
    int id_sem_ricezione;
    int id_shm_ricezione;

} msg_init_response;


#endif