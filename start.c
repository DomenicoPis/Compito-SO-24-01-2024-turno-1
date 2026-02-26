#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "header_msg.h"

int main() {


    /* TBD: Creare una coppia di code di messaggi UNIX */

    int chiave_req = ftok(".", 'a');
    int id_coda_req = msgget(chiave_req, IPC_CREAT | 0664);
    if(id_coda_req < 0){
        perror("errore msgget invio");
        exit(1);
    }

    int chiave_res = ftok(".", 'b');
    int id_coda_res = msgget(chiave_res, IPC_CREAT | 0664);
    if(id_coda_res < 0){
        perror("errore msgget risposta");
        exit(1);
    }

    pid_t pid;

    /* TBD: Creare un processo figlio, che esegua l'eseguibile "server" */

    pid = fork();

    if(pid == 0){
        execl("./server", "server", NULL);
        perror("errore exec server");
        exit(1);
    }else if(pid < 0){
        perror("errore fork server");
        exit(1);
    }


    /* TBD: Creare 3 processi figli, che eseguano l'eseguibile "client" */

    for(int i=0; i<3; i++){
        pid = fork();

        if(pid == 0){
            execl("./client", "client", NULL);
            perror("errore exec client");
            exit(1);
        }else if(pid < 0){
            perror("errore fork client");
            exit(1);
        }

    }


    /* TBD: Attendere la terminazione dei figli, deallocare le code di messaggi */

    int status;

    for(int i=0; i<4; i++){
        wait(&status);
    }

    msgctl(id_coda_req, IPC_RMID, NULL);
    msgctl(id_coda_res, IPC_RMID, NULL);

    return 0;
}