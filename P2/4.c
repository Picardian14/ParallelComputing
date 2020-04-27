#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

int *vector, N, T, min=999, max=0; //variables que deben conocer todos los threads
sem_t min_sem, max_sem;

void * minmax(void *ptr){
    int *p, id;
    p = (int*) ptr;
    id = *p;
    int local_min=999, local_max=0;
    int s, e;
    int L = N / T; //secciono segun la canitdad de threads
    s = id*L;
    e = (N % 2 ==1 && id == T-1) ? ((id+1)*L)+1 : (id+1)*L;
    for(int k=s; k<e;k++){
        if (local_min > vector[k])
            local_min = vector[k];
        if (local_max < vector[k])
            local_max = vector[k];
    }
    sem_wait(&min_sem);
    if (local_min < min)
        min = local_min;
    sem_post(&min_sem);
    sem_wait(&max_sem);
    if (local_max > max)
        max = local_max;
    sem_post(&max_sem);
}


int main(int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "FALTAN ARGUMENTOS. [N] [T]");
    }
    else{
        N=atoi(argv[1]);
        T=atoi(argv[2]);        
    }
    
    pthread_attr_t attr;
    pthread_t threads[T];
    int ids[T];
    pthread_attr_init(&attr);
    sem_init(&min_sem, 0, 1);
    sem_init(&max_sem, 0, 1);

    vector = (int *)malloc(sizeof(int)*N);
    long b;
    for(int i=0; i < N; i++){
        srandom(i+N);
        b = random();
        vector[i] = b % 100;
        printf("%d ", vector[i]);
    }
    for(int t=0; t<T; t++){
        ids[t] = t;
        pthread_create(&threads[t], &attr, minmax, &ids[t]);
    }
    for(int t=0; t<T; t++){        
        pthread_join(threads[t], NULL);
    }
    
    printf("\n");
    printf("Minimo = %d\nMaximo = %d", min, max);
    printf("\n");

    sem_destroy(&min_sem);
    sem_destroy(&max_sem);
    return 0;
}