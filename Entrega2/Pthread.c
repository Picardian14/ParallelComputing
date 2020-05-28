#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int N, T;
double *A, *B, *C, *D, *temp_AB;
int a_cant, b_cant, c_cant;
double a_min, b_min, c_min;
double a_max, b_max, c_max;
double a_avg, b_avg, c_avg;
double scalar;

pthread_mutex_t a_cant_mutex;
pthread_mutex_t a_min_mutex, a_max_mutex;



pthread_barrier_t barrier;

double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}


void * min_max_avg(void *ptr)
{
    int *p, id;
    p = (int*) ptr;
    id = *p;
    int a_local_cant, b_local_cant, c_local_cant = 0;
    int a_local_min=999, a_local_max=0;
    int b_local_min=999, b_local_max=0;
    int c_local_min=999, c_local_max=0;
    int s, e;
    int L = N / T; //secciono segun la canitdad de threads
    s = id*L;
    e = (N % 2 ==1 && id == T-1) ? ((id+1)*L)+1 : (id+1)*L;
    for(int i=0; i < N ; i++)
    {
        for(int j=s; j<e;j++)
        {
            a_local_cant+= A[i*N+j];
            if (a_local_min > A[i*N+j])
                a_local_min = A[i*N+j];
            if (a_local_max < A[i*N+j])
                a_local_max = A[i*N+j];
        }
    }
    for(int i=0; i < N ; i++)
    {
        for(int j=s; j<e;j++)
        {
            b_local_cant+= B[i*N+j];
            if (b_local_min > B[i*N+j])
                b_local_min = B[i*N+j];
            if (b_local_max < B[i*N+j])
                b_local_max = B[i*N+j];
        }
    }
    for(int i=0; i < N ; i++)
    {
        for(int j=s; j<e;j++)
        {
            c_local_cant+= C[i*N+j];
            if (c_local_min > C[i*N+j])
                c_local_min = C[i*N+j];
            if (c_local_max < C[i*N+j])
                c_local_max = C[i*N+j];
        }
    }
    pthread_mutex_lock(&a_cant_mutex);
    a_cant+=a_local_cant;
    b_cant+=b_local_cant;    
    c_cant+=c_local_cant;
    pthread_mutex_unlock(&a_cant_mutex);

    pthread_mutex_lock(&a_min_mutex);
    if (a_local_min < a_min)
        a_min = a_local_min;        
    if (b_local_min < b_min)
        b_min = b_local_min;
    if (c_local_min < c_min)
        c_min = c_local_min;    
    pthread_mutex_unlock(&a_min_mutex);

    pthread_mutex_lock(&a_max_mutex);
    if (a_local_max > a_max)
        a_max = a_local_max;    
    if (b_local_max > b_max)
        b_max = b_local_max;
    if (c_local_max > c_max)
        c_max = c_local_max;
    pthread_mutex_unlock(&a_max_mutex);

    pthread_barrier_wait(&barrier);
    if(id == 0)
    {
        a_avg = (double)a_cant/(double)N;
        b_avg = (double)b_cant/(double)N;
        c_avg = (double)c_cant/(double)N;
        double diff_temp = (a_max*b_max*c_max - a_min*b_min*c_min);
        double avg_temp =(a_avg*b_avg*c_avg);
        scalar = diff_temp/avg_temp;
    }
    pthread_barrier_wait(&barrier);
    for(int i=s; i < e ; i++)
    {
        for(int j=0; j<N;j++)
        {
            for(int k=0;k < N; k++)
                temp_AB[N*i+j] += A[N*i+k]*B[N*j+k];
        }
    }
    for(int i=s; i < e ; i++)
    {
        for(int j=0; j<N;j++)
        {
            for(int k=0;k < N; k++)
                D[N*i+j] += temp_AB[N*i+k]*C[N*j+k];            
        }
    }
    for(int i=s; i < e ; i++)
    {
        for(int j=0; j<N;j++)
        {            
            D[N*i+j] *= scalar;
        }
    }

}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("MISSING ARGUMENTS. Usage [N] [T]");
        exit(0);
    }
    N=atoi(argv[1]);
    T=atoi(argv[2]);
    a_min, b_min, c_min = 999;
    a_max, b_max, c_max = 0;
    a_cant, b_cant, c_cant = 0;
    
    double ticktick;
    pthread_attr_t attr;
    pthread_t threads[T];
    int ids[T];
    pthread_attr_init(&attr);
    pthread_mutex_init(&a_cant_mutex, NULL);    
    pthread_mutex_init(&a_min_mutex, NULL);
    pthread_mutex_init(&a_max_mutex, NULL);
    

    pthread_barrier_init(&barrier, NULL, T);

    A=(double*)malloc(sizeof(double)*N*N);
    B=(double*)malloc(sizeof(double)*N*N);
    C=(double*)malloc(sizeof(double)*N*N);
    D=(double*)malloc(sizeof(double)*N*N);
    temp_AB=(double*)malloc(sizeof(double)*N*N);
    for(int row=0; row < N; row++){
        for(int col=0 ; col < N; col++){
            if (row == col){
                A[N*row+col] = 1.0;
                C[N*row+col] = 1.0;
            }else{
                A[N*row+col] = 0;
                C[N*row+col] = 0;
            }
            B[N*row+col] = 2.0;
        }
    }
    ticktick = dwalltime();
    for(int t=0; t<T; t++)
    {
        ids[t] = t;
        pthread_create(&threads[t], &attr, min_max_avg, &ids[t]);
    }
    for(int t=0; t<T; t++)
    {
        pthread_join(threads[t], NULL);
    }
    
    /*for(int t=0; t<T; t++)
    {
        ids[t] = t;
        pthread_create(&threads[t], &attr, product, &ids[t]);
    }
    for(int t=0; t<T; t++)
    {
        pthread_join(threads[t], NULL);
    }*/
    ticktick = dwalltime() - ticktick;
    printf("Tiempo de ejecucion: %f", ticktick);
    free(A);
    free(B);
    free(C);
    free(D);
    return 0;
    
}