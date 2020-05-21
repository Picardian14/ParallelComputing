#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define COORDINATOR 0

int main(int argc, char* argv[]){
	int i, j, k, numProcs, rank, n, stripSize, check=1;
	double * A,*A_B , *B, *C,*D, *local_max, *local_min, *local_avg, *max, *min, *avg, escalar;
	MPI_Status status;
	double commTimes[4], maxCommTimes[4], minCommTimes[4], commTime, totalTime;

	/* Lee par�metros de la l�nea de comando */
	if ((argc != 2) || ((n = atoi(argv[1])) <= 0) ) {
	    printf("\nUsar: %s size \n  size: Dimension del vector\n", argv[0]);
		exit(1);
	}

	MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	if (n % numProcs != 0) {
		printf("El tama�o de la matriz debe ser multiplo del numero de procesos.\n");
		exit(1);
	}

	// calcular porcion de cada worker
	stripSize = n / numProcs;

	// Reservar memoria
	if (rank == COORDINATOR) {
        max = (double*) malloc(sizeof(double)*3);
        min = (double*) malloc(sizeof(double)*3);
        avg = (double*) malloc(sizeof(double)*3);
		A = (double*) malloc(sizeof(double)*n*n);
        A_B = (double*) malloc(sizeof(double)*n*n);
        D = (double*) malloc(sizeof(double)*n*n);
	}
	else  {
		A = (double*) malloc(sizeof(double)*stripSize*n);
        A_B = (double*) malloc(sizeof(double)*n*stripSize);
        D = (double*) malloc(sizeof(double)*stripSize*n);
	}
	
	B = (double*) malloc(sizeof(double)*n*n);
    C = (double*) malloc(sizeof(double)*n*n);
    local_max = (double*) malloc(sizeof(double)*3);
    local_min = (double*) malloc(sizeof(double)*3);
    local_avg = (double*) malloc(sizeof(double)*3);

    //TODOSOS inicializan los minimos/maximos/promedios locales
    for (i=0; i<3; i++){
        local_max[i]=-1.0;
        local_min[i]=101;
        local_avg[i]=0;
    }
	// inicializar datos
	if (rank == COORDINATOR){ 
        printf('max: %lf',local_max[0]);
        for(i=0;i<(n*n);i++){
	        A[i]=0.0;
            B[i]=0.0;
            C[i]=0.0;
            D[i]=0.0;
            A_B[i]=0.0;
        }
 //seteo las diagonales en 1 y tengo la matriz identidad
    for (i=0; i<n; i++){
	     B[i*n+i]= 1.0;
         C[i*n+i]= 1.0;
         A[i*n+i]= 2.0;
     }
    }
	MPI_Barrier(MPI_COMM_WORLD);

	commTimes[0] = MPI_Wtime();

	// 1ra distribucion de datos
    //MPI_Bcast(B, n*n, MPI_INT, COORDINATOR, MPI_COMM_WORLD)
    //MPI_Bcast(C, n*n, MPI_INT, COORDINATOR, MPI_COMM_WORLD)
    MPI_Scatter(B, stripSize*n, MPI_DOUBLE, B, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(C, stripSize*n, MPI_DOUBLE, C, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(A, stripSize*n, MPI_DOUBLE, A, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //MPI_Scatter(A_B, stripSize*n, MPI_INT, A_B, stripSize*n, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Scatter(D, stripSize*n, MPI_INT, D, stripSize*n, MPI_INT, 0, MPI_COMM_WORLD);
	commTimes[1] = MPI_Wtime();

	/* computo de los maximos, minimos y promedios*/
    //computo de A
	for (i=0; i<n*stripSize; i++){
        if (local_min[0] > A[i])
            local_min[0] = A[i];
        if (local_max[0] < A[i])
            local_max[0] = A[i];
        local_avg[0] += A[i]; 
	}
     //computo de B
	for (i=0; i<n*stripSize; i++){
        if (local_min[1] > B[i])
            local_min[1] = B[i];
        if (local_max[1] < B[i])
            local_max[1] = B[i];
        local_avg[1] += B[i]; 
	}
    //computo de C
	for (i=0; i<n*stripSize; i++){
        if (local_min[2] > C[i])
            local_min[2] = C[i];
        if (local_max[2] < C[i])
            local_max[2] = C[i];
        local_avg[2] += C[i]; 
	}
    
 
    MPI_Reduce(&local_min[0], &min[0], 1, MPI_DOUBLE, MPI_MIN, COORDINATOR , MPI_COMM_WORLD);
    MPI_Reduce(&local_min[1], &min[1], 1, MPI_DOUBLE, MPI_MIN, COORDINATOR , MPI_COMM_WORLD);
    MPI_Reduce(&local_min[2], &min[2], 1, MPI_DOUBLE, MPI_MIN, COORDINATOR , MPI_COMM_WORLD);
    MPI_Reduce(&local_max[0], &max[0], 1, MPI_DOUBLE, MPI_MAX, COORDINATOR , MPI_COMM_WORLD);
    MPI_Reduce(&local_max[1], &max[1], 1, MPI_DOUBLE, MPI_MAX, COORDINATOR , MPI_COMM_WORLD); 
    MPI_Reduce(&local_max[2], &max[2], 1, MPI_DOUBLE, MPI_MAX, COORDINATOR , MPI_COMM_WORLD); 
    MPI_Reduce(&local_avg[0], &avg[0], 1, MPI_DOUBLE, MPI_SUM, COORDINATOR , MPI_COMM_WORLD); 
    MPI_Reduce(&local_avg[1], &avg[1], 1, MPI_DOUBLE, MPI_SUM, COORDINATOR , MPI_COMM_WORLD);
    MPI_Reduce(&local_avg[2], &avg[2], 1, MPI_DOUBLE, MPI_SUM, COORDINATOR , MPI_COMM_WORLD);
    //calculamos el escalar
    if (rank==COORDINATOR) {
        avg[0] = avg[0]/n*n;
        avg[1] = avg[1]/n*n;
        avg[2] = avg[2]/n*n;
        escalar = ((max[0]*max[1]*max[2])-(min[0]*min[1]*min[2]))/(avg[0]*avg[1]*avg[2]);
        printf('escalar: %f',escalar);
    }  


	MPI_Finalize();

	
	free(A);free(A_B);free(B);free(C);free(D);

	return 0;
}