#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define COORDINATOR 0

int main(int argc, char* argv[]){
	int i, j, k, numProcs, rank, n, stripSize, check=1, provided;
	double * A,*A_B , *B, *C,*D, local_max[3], local_min[3], local_avg[3], max[3], min[3], avg[3], escalar;
	MPI_Status status;
	double commTimes[8], maxCommTimes[8], minCommTimes[8], commTime, totalTime, procTime;

	/* Lee par�metros de la l�nea de comando */
	if ((argc != 2) || ((n = atoi(argv[1])) <= 0) ) {
	    printf("\nUsar: %s size \n  size: Dimension del vector\n", argv[0]);
		exit(1);
	}

	MPI_Init_thread(&argc,&argv, MPI_THREAD_MULTIPLE, &provided);

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

    //TODOSOS inicializan los minimos/maximos/promedios locales
    for (i=0; i<3; i++){
        local_max[i]=-1.0;
        local_min[i]=101;
        local_avg[i]=0;
    }
	// inicializar datos
	if (rank == COORDINATOR){ 
        //printf('max: %lf',local_max[0]);
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
    
    MPI_Bcast(B, n*n, MPI_DOUBLE, COORDINATOR, MPI_COMM_WORLD);
    MPI_Bcast(C, n*n, MPI_DOUBLE, COORDINATOR, MPI_COMM_WORLD);
    MPI_Scatter(A_B, stripSize*n, MPI_DOUBLE, A_B, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(D, stripSize*n, MPI_DOUBLE, D, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(A, stripSize*n, MPI_DOUBLE, A, stripSize*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    
	commTimes[1] = MPI_Wtime();

    #pragma omp parallel private(i) 
    { 
        /* computo de los maximos, minimos y promedios*/
        //computo de A
        #pragma omp for reduction(min:local_min[0]) reduction(max:local_max[0]) reduction(+:local_avg[0]) schedule(static)
        for(i=0;i<stripSize*n;i++){
            if (local_min[0] > A[i])
                local_min[0] = A[i];
            if (local_max[0] < A[i])
                local_max[0] = A[i];

            local_avg[0] += A[i];
        }
        
        #pragma omp for reduction(min:local_min[1]) reduction(max:local_max[1]) reduction(+:local_avg[1]) schedule(static)
        for(i=(rank*stripSize*n); i<(rank+1)*stripSize*n; i++){
            if (local_min[1] > B[i])
                local_min[1] = B[i];
            if (local_max[1] < B[i])
                local_max[1] = B[i];
            
            local_avg[1] += B[i];
        }

        #pragma omp for reduction(min:local_min[2]) reduction(max:local_max[2]) reduction(+:local_avg[2]) schedule(static)
        for(i=(rank*stripSize*n); i<(rank+1)*stripSize*n; i++){
            if (local_min[2] > C[i])
                local_min[2] = C[i];
            if (local_max[2] < C[i])
                local_max[2] = C[i];

            local_avg[2] += C[i];
        }
    }
    local_avg[0] = local_avg[0]/(n*n);
    local_avg[1] = local_avg[1]/(n*n);
    local_avg[2] = local_avg[2]/(n*n);
    commTimes[2] = MPI_Wtime();

    MPI_Allreduce(&local_min[0], &min[0], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_min[1], &min[1], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_min[2], &min[2], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_max[0], &max[0], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&local_max[1], &max[1], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD); 
    MPI_Allreduce(&local_max[2], &max[2], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD); 
    MPI_Allreduce(&local_avg[0], &avg[0], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
    MPI_Allreduce(&local_avg[1], &avg[1], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&local_avg[2], &avg[2], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    commTimes[3] = MPI_Wtime();
    //calculamos el escalar

    escalar = ((max[0]*max[1]*max[2])-(min[0]*min[1]*min[2]))/(avg[0]*avg[1]*avg[2]);
    if(rank==0){
        printf("B:Max: %lf \tMin: %lf \tAvg: %lf \t\n", max[1], min[1], avg[1]);
        printf("C:Max: %lf \tMin: %lf \tAvg: %lf \t\n", max[2], min[2], avg[2]);
        printf("A:Max: %lf \tMin: %lf \tAvg: %lf \t\n", max[0], min[0], avg[0]);
        printf("Escalar %f\n\n",escalar);
    }
    
    commTimes[4] = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    //2da distribucion de datos (A ya fue distribuido)

    commTimes[5] = MPI_Wtime();
    #pragma omp parallel private(i,j,k)
    { 
        //Computacion parcial
        #pragma omp for schedule(static)
        for (i=0; i<stripSize; i++) {
            for (j=0; j<n ;j++ ) {
                A_B[i*n+j]=0;
                for (k=0; k<n ;k++ ) { 
                    A_B[i*n+j] += (A[i*n+k]*B[j*n+k]); 
                }
            }
        }
        #pragma omp for schedule(static)
        for (i=0; i<stripSize; i++) {
            for (j=0; j<n ;j++ ) {
                D[i*n+j]=0;
                for (k=0; k<n ;k++ ) { 
                    D[i*n+j] += (A_B[i*n+k]*C[j*n+k]); 
                }
                D[i*n+j] *= escalar;
            }
        }
    }

    commTimes[6] = MPI_Wtime();
    
    MPI_Gather(D, stripSize*n, MPI_DOUBLE, D, stripSize*n, MPI_DOUBLE, COORDINATOR, MPI_COMM_WORLD);

    commTimes[7] = MPI_Wtime();

    MPI_Reduce(commTimes, minCommTimes, 8, MPI_DOUBLE, MPI_MIN, COORDINATOR, MPI_COMM_WORLD);
	MPI_Reduce(commTimes, maxCommTimes, 8, MPI_DOUBLE, MPI_MAX, COORDINATOR, MPI_COMM_WORLD);

    MPI_Finalize();

    if(rank == COORDINATOR){
        printf("Comprobando Resultado\n");
        for (i=0; i<n*n; i++) {
            if(D[i] != A[i]*escalar){
                check=0;
                break;
            }
        }
        totalTime = maxCommTimes[7] - minCommTimes[0];
        commTime = (maxCommTimes[1] - minCommTimes[0]) + (maxCommTimes[3] - minCommTimes[2]) + (maxCommTimes[5] - minCommTimes[4]) + (maxCommTimes[7] - minCommTimes[6]);		
        procTime = (maxCommTimes[2] - minCommTimes[1]) + (maxCommTimes[4] - minCommTimes[3]) + (maxCommTimes[6] - minCommTimes[5]);    
        if(check){
            printf("Resultados Correctos\n");
            printf("Multiplicacion de matrices (N=%d)\tTiempo total=%lf\tTiempo comunicacion=%lf\tTiempo De Procesamiento=%lf\n",n,totalTime,commTime, procTime);
        }
        else
            printf("Resultados InCorrectos\n");
        
    }
    

	

	
	free(A);free(A_B);free(B);free(C);free(D);

	return 0;
}
