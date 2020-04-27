#include <pthread.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

int *vector, N, T, cant, X; //variables que deben conocer todos los threads
pthread_mutex_t cant_mutex;

void * findX(void *ptr){
    int *p, id;
    p = (int*) ptr;
    id = *p;
    int localCant;
    int s, e;
    int L = N / T; //secciono segun la canitdad de threads
    s = id*L;
    e = (N % 2 ==1 && id == T-1) ? ((id+1)*L)+1 : (id+1)*L;
    for(int k=s; k<e;k++){
        localCant += (vector[k]==X) ? 1 : 0;
    }
    pthread_mutex_lock(&cant_mutex);
    cant+=localCant;
    pthread_mutex_unlock(&cant_mutex);
}


int main(int argc, char *argv[]){

    if (argc < 4){
        fprintf(stderr, "FALTAN ARGUMENTOS. [N] [T] [X]");
    }
    else{
        N=atoi(argv[1]);
        T=atoi(argv[2]);
        X=atoi(argv[3]);
    }
    cant=0;
    pthread_attr_t attr;
    pthread_t threads[T];
    int ids[T];
    pthread_attr_init(&attr);
    pthread_mutex_init(&cant_mutex, NULL);

    vector = (int *)malloc(sizeof(int)*N);
    long b;
    for(int i=0; i < N; i++){
        srandom(i+X);
        b = random();
        vector[i] = b % 10;
    }
    for(int t=0; t<T; t++){
        ids[t] = t;
        pthread_create(&threads[t], &attr, findX, &ids[t]);
    }
    for(int t=0; t<T; t++){        
        pthread_join(threads[t], NULL);
    }
    for(int i=0; i < N; i++){
        printf("%d ", vector[i]);
        
    }
    printf("\nOcrurrencias de %d : %d", X, cant);
    
    return 0;
}