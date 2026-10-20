#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
 
#define N 5
#define NUM_CLIENTI 10
 
pthread_mutex_t mt = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
 
int *buffer;
int in = 0, out = 0;
int count = 0;
 
void *cliente (void *arg){
 
    int id = *(int *)arg;
 
    pthread_mutex_lock(&mt);
    while(count >= N){
        pthread_cond_wait(&not_full, &mt);
    }
 
    buffer[in];
    printf("Il cliente %d attende di essere servito, posizione %d (STATO CONTATORE: %d)\n",
            id, in, count+1);
 
    count++;
    in = (in + 1) % N;
 
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mt);
    usleep(40000 + rand()%60000);
 
    return NULL;
}
 
void *negoziante (void *arg){
 
    for(int i =0; i < NUM_CLIENTI; i++){
        pthread_mutex_lock(&mt);
        while(count <= 0){
            pthread_cond_wait(&not_empty, &mt);
        }
 
        buffer[out];
        printf("Il negoziante ha servito il cliente in posizione %d (STATO CONTATORE: %d)\n",
                out, count-1);
 
        out = (out + 1) % N;
        count--;
 
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mt);
        usleep(20000 + rand()%20000);
    }
 
    return NULL;
}
 
int main(){
    pthread_t clienti[NUM_CLIENTI];
    pthread_t negoz;
 
    int id[NUM_CLIENTI];
 
    srand(time(NULL));
 
    buffer= malloc(N * sizeof(int));
 
    for(int i = 0; i < NUM_CLIENTI; i++){
        id[i] =i;
        pthread_create(&clienti[i], NULL, cliente, &id[i]);
    }
 
    pthread_create(&negoz, NULL, negoziante, NULL);
 
    for(int i = 0; i < NUM_CLIENTI; i++){
        pthread_join(clienti[i], NULL);
    }
 
    pthread_join(negoz, NULL);
 
    free(buffer);
    pthread_mutex_destroy(&mt);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
 
    return 0;
}