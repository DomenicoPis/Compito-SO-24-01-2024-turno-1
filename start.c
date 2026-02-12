#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "header_msg.h"

int main() {


    /* TBD: Creare una coppia di code di messaggi UNIX */

    key_t chiave_invio = ftok(".", 'a');
    if (chiave_invio ==-1){ 
        perror("Errore ftok chiave invio"); 
        exit(1); 
    }

    int id_coda_invio = msgget(chiave_invio, IPC_CREAT | 0664);
    if (id_coda_invio < 0){ 
        perror("Errore msgget coda invio"); 
        exit(1); 
    }

    key_t chiave_risposta = ftok(".", 'b');
    if (chiave_risposta ==-1){ 
        perror("Errore ftok chiave risposta"); 
        exit(1); 
    }

    int id_coda_risposta = msgget(chiave_risposta, IPC_CREAT | 0664);
    if(id_coda_risposta < 0){
        perror("errore msgget coda risposta");
        exit(1);
    }


    /* TBD: Creare un processo figlio, che esegua l'eseguibile "server" */

    pid_t pid = fork();

    if(pid == 0){

        execl("./server", "server", NULL);
        
        // Se arrivo qui, execl ha fallito
        perror("Errore exec server");
        exit(1);
    }


    /* TBD: Creare 3 processi figli, che eseguano l'eseguibile "client" */

    for(int i=0; i<3; i++){

        pid_t pid = fork();

        if(pid == 0){

           execl("./client", "client", NULL);
            
            perror("Errore exec client");
            exit(1);
        }

    }


    /* TBD: Attendere la terminazione dei figli, deallocare le code di messaggi */

    int status;

    for (int i=0; i<4; i++){

        wait(&status);

    }

    msgctl(id_coda_invio, IPC_RMID, NULL);
    msgctl(id_coda_risposta, IPC_RMID, NULL);

    return 0;
}