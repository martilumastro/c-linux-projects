#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>

int main(void){
    int funzionePidPadre = getpid();
    int systemCallPidPadre = syscall(SYS_getpid);
    printf("Processo padre PID (tramite funzione getpid()): %d\n", funzionePidPadre);
    printf("Processo padre: PID (tramite richiesta esplicita con system call SYS_getpid): %d\n", systemCallPidPadre);
    fflush(stdout);
    pid_t pid = fork();
    if(pid < 0){
        perror("Errore nella fork");
        return 1;
    } else if(pid == 0){
        int funzionePidFiglio = getpid();
        int systemCallPidFiglio = syscall(SYS_getpid);
        printf("Processo figlio PID (tramite funzione getpid()): %d\n", funzionePidFiglio);
        printf("Processo figlio: PID (tramite richiesta esplicita con system call SYS_getpid): %d\n", systemCallPidFiglio);
        fflush(stdout);
    }
    
    int status;
    wait(&status);
    if(WIFEXITED(status)){
        printf("Il processo figlio è terminato con codice di uscita: %d\n", WEXITSTATUS(status));
    } else {
        printf("Il processo figlio non è terminato correttamente\n");
        return 1;
    }
    return 0;
}