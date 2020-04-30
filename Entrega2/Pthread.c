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
double scalar;

pthread_mutex_t a_cant_mutex, b_cant_mutex, c_cant_mutex;
sem_t a_min_sem, a_max_sem;
sem_t b_min_sem, b_max_sem;
sem_t c_min_sem, c_max_sem;

double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

void * product (void* ptr)
{
    int *p, id;
    p = (int *) ptr;
    id = *p;
    int row, col, start, end;
    int L = N / T;
    start = id*L;
    end = (id+1)*L;
    for(row=start; row < end; row++)
    {
        for(col=0 ; col < N; col++)
        {
            for(int k=0;k < N; k++)
                temp_AB[N*row+col] += A[N*row+k]*B[N*col+k];
        }
    }
    for(row=start; row < end; row++)
    {
        for(col=0 ; col < N; col++)
        {
            for(int k=0;k < N; k++)
                D[N*row+col] += temp_AB[N*row+k]*C[N*col+k];
            D[N*row+col] *= scalar;
        }
    }
    
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
            b_local_cant+= B[i*N+j];
            if (b_local_min > B[i*N+j])
                b_local_min = B[i*N+j];
            if (b_local_max < B[i*N+j])
                b_local_max = B[i*N+j];
            c_local_cant+= C[i*N+j];
            if (c_local_min > C[i*N+j])
                c_local_min = C[i*N+j];
            if (c_local_max < C[i*N+j])
                c_local_max = C[i*N+j];
        }
    }

    pthread_mutex_lock(&a_cant_mutex);
    a_cant+=a_local_cant;
    pthread_mutex_unlock(&a_cant_mutex);

    pthread_mutex_lock(&b_cant_mutex);
    b_cant+=b_local_cant;    
    pthread_mutex_unlock(&b_cant_mutex);

    pthread_mutex_lock(&c_cant_mutex);
    c_cant+=c_local_cant;
    pthread_mutex_unlock(&c_cant_mutex);

    sem_wait(&a_min_sem);
    if (a_local_min < a_min)
        a_min = a_local_min;
    sem_post(&a_min_sem);
    sem_wait(&a_max_sem);
    if (a_local_max > a_max)
        a_max = a_local_max;
    sem_post(&a_max_sem);

    sem_wait(&b_min_sem);
    if (b_local_min < b_min)
        b_min = b_local_min;
    sem_post(&b_min_sem);
    sem_wait(&b_max_sem);
    if (b_local_max > b_max)
        b_max = b_local_max;
    sem_post(&b_max_sem);

    sem_wait(&c_min_sem);
    if (c_local_min < c_min)
        c_min = c_local_min;
    sem_post(&c_min_sem);
    sem_wait(&c_max_sem);
    if (c_local_max > c_max)
        c_max = c_local_max;
    sem_post(&c_max_sem);

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
    double a_avg, b_avg, c_avg;
    double ticktick;
    pthread_attr_t attr;
    pthread_t threads[T];
    int ids[T];
    pthread_attr_init(&attr);
    pthread_mutex_init(&a_cant_mutex, NULL);
    pthread_mutex_init(&b_cant_mutex, NULL);
    pthread_mutex_init(&c_cant_mutex, NULL);
    sem_init(&a_min_sem, 0, 1);
    sem_init(&a_max_sem, 0, 1);
    sem_init(&b_min_sem, 0, 1);
    sem_init(&b_max_sem, 0, 1);
    sem_init(&c_min_sem, 0, 1);
    sem_init(&c_max_sem, 0, 1);

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
    a_avg = (double)a_cant/(double)N;
    b_avg = (double)b_cant/(double)N;
    c_avg = (double)c_cant/(double)N;
    double diff_temp = (a_max*b_max*c_max - a_min*b_min*c_min);
    double avg_temp =(a_avg*b_avg*c_avg);
    scalar = diff_temp/avg_temp;
    for(int t=0; t<T; t++)
    {
        ids[t] = t;
        pthread_create(&threads[t], &attr, product, &ids[t]);
    }
    for(int t=0; t<T; t++)
    {
        pthread_join(threads[t], NULL);
    }
    ticktick = dwalltime() - ticktick;
    printf("Tiempo de ejecucion: %f", ticktick);
    return 0;
    
}