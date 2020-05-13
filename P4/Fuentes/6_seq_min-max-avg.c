#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/time.h>

double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec/1000000.0;
	return sec;
}

int main(int argc, char * argv[])
{
    int N, min=453, max=-1, sum=0, i;
    int *vector;
    double tick;

    if (argc < 2)
    {
        printf("FALTA TAMANIO DEL VECTOR");
        exit(0);
    }
    N=atoi(argv[1]);
    vector = malloc(sizeof(int)*N);
    for (i=0;i<N;i++)
        vector[i]=rand()%452+rand()%275;
    tick = dwalltime();
    for (i=0; i<N;i++)
    {
        if(vector[i]>max)
            max=vector[i];
        if(vector[i]<min)
            min=vector[i];
        sum+=vector[i];
    }
    tick = dwalltime() - tick;
    printf("Tiempo de procesamiento: %f", tick);
    return 0;
}



