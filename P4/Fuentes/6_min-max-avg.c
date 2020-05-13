#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define COORDINATOR 0

int main(int argc, char *argv[])
{
    int N, numProcs, rank, stripsize;
    int *vector;
    int localmax=-1, localmin=401, localsum=0, max=-1, min=401, sum=0;
    double avg;
    double commTimes[4], maxCommTimes[4], minCommTimes[4], commTime, totalTime;
    if (argc < 2)
    {
        printf("FALTA TAMANIO DEL VECTOR");
        exit(0);
    }
    N=atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    stripsize = N / numProcs;

    vector = malloc(sizeof(int)*N);
    if (rank == COORDINATOR)
        for (int i=0; i<N; i++)
            vector[i] =rand()%561 + rand()%400;

    
    MPI_Barrier(MPI_COMM_WORLD);

    commTimes[0] = MPI_Wtime();
    
    MPI_Scatter(vector, stripsize, MPI_INT, vector, stripsize, MPI_INT, COORDINATOR, MPI_COMM_WORLD);

    commTimes[1] = MPI_Wtime();

    for (int j=0; j<stripsize; j++){
        if (vector[j] < localmin)
            localmin=vector[j];
        if (vector[j] > localmax)
            localmax=vector[j];
        localsum+=vector[j];
    }

    commTimes[2] = MPI_Wtime();

    MPI_Reduce(&localmin, &min, 1, MPI_INT, MPI_MIN, COORDINATOR,MPI_COMM_WORLD);
    MPI_Reduce(&localmax, &max, 1, MPI_INT, MPI_MAX, COORDINATOR,MPI_COMM_WORLD);
    MPI_Reduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, COORDINATOR,MPI_COMM_WORLD);
    commTimes[3] = MPI_Wtime();

    MPI_Reduce(commTimes, minCommTimes, 4, MPI_DOUBLE, MPI_MIN, COORDINATOR, MPI_COMM_WORLD);
	MPI_Reduce(commTimes, maxCommTimes, 4, MPI_DOUBLE, MPI_MAX, COORDINATOR, MPI_COMM_WORLD);
    MPI_Finalize();
    
    if (rank == COORDINATOR)
    {
        totalTime = maxCommTimes[3] - minCommTimes[0];
        commTime = (maxCommTimes[1] - minCommTimes[0]) + (maxCommTimes[3] - minCommTimes[2]);		
        avg = (double) sum / (double) N;
        printf("Busqueda en vecotr (N=%d)\tTiempo total=%lf\tTiempo comunicacion=%lf\n",N,totalTime,commTime);
        printf("Min = %d\tMax = %d\tAvg = %.2f",min,max,avg);
    }
    return 0;
}