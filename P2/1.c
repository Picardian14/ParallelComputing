#include <pthread.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
double *A, *B, *C;
int N, T;
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}
void * product (void* ptr){
    int *p, id;
    p = (int *) ptr;
    id = *p;
    int row, col, start, end;
    int L = N / T;
    start = id*L;
    end = (id+1)*L;
    for(row=start; row < end; row++){
        for(col=0 ; col < N; col++){
            for(int k=0;k < N; k++)
            C[N*row+col] += A[N*row+k]*B[N*col+k];
        }
    }
}

int main (int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "FALTAN ARGUMENTOS. [N={256, 512, 1024...}] [T={2,4}]");
    }
    else{
        N=atoi(argv[1]);
        T=atoi(argv[2]);        
    }
    int ids[T];
    int row, col;
    pthread_attr_t attr;
    pthread_t threads[T];

    pthread_attr_init(&attr);
    
    //inicializo matrices bidimensionales
    A=(double*)malloc(sizeof(double)*N*N);
    B=(double*)malloc(sizeof(double)*N*N);
    C=(double*)malloc(sizeof(double)*N*N);
    for(row=0; row < N; row++){
        for(col=0 ; col < N; col++){
            if (row == col){
                A[N*row+col] = 1;
            }else{
                A[N*row+col] = 0;
            }
            B[N*row+col] = 1;
        }
    }
    double t = dwalltime();
    for (int i=0; i<T; i++){
        ids[i] = i;
        pthread_create(&threads[i], &attr, product, &ids[i]);
    }
    for (int i=0; i<T; i++){        
        pthread_join(threads[i], NULL);
    }
    double time = dwalltime() - t;
    printf("Tiempo con algoritmo basico paralelizad: %f", time);
    
    for(row=0; row < N; row++){
        for(col=0 ; col < N; col++){
            if (col == N-1){
                printf("%.1f \n", C[N*row+col]);
            }
            else{
                printf("%.1f ", C[N*row+col]);
            }
        }
    }

    


    

    return 0;
}

