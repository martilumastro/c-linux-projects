#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main(){
    char src[100], dest[100];
    int fd_src, fd_dest;
    ssize_t n;
    char buffer[128];
    printf("Inserisci file sorgente: ");
    if(fgets(src, sizeof(src), stdin) == NULL){
        perror("Errore nella lettura del file sorgente");
        exit(EXIT_FAILURE);
    }
    src[strcspn(src, "\n")] = '\0';

    printf("Inserisci file destinazione: ");
    if(fgets(dest, sizeof(dest), stdin) == NULL){
        perror("Errore nella lettura del file destinazione");
        exit(EXIT_FAILURE);
    }
    dest[strcspn(dest, "\n")] = '\0';

    if(access(src, R_OK) != 0){
        perror("Errore nell'accesso al file sorgente");
        exit(EXIT_FAILURE);
    }

    fd_src = open(src, O_RDONLY);
    if(fd_src < 0){
        perror("Errore nell'apertura del file sorgente");
        exit(EXIT_FAILURE);
    }

    fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd_dest < 0){
        perror("Errore nell'apertura del file destinazione");
        close(fd_src);
        exit(EXIT_FAILURE);
    }

    while((n = read(fd_src, buffer, sizeof(buffer))) > 0){
        if(write(fd_dest, buffer, n) != n){
            perror("Errore nella scrittura del file destinazione");
            close(fd_src);
            close(fd_dest);
            exit(EXIT_FAILURE);
        }
    }
    printf("Copia completata con successo.\n");
    close(fd_src);
    close(fd_dest);
    return 0;
}