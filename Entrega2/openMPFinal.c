#include<stdio.h>
#include<stdlib.h>
#include<omp.h>


//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

int main(int argc,char*argv[]){

 double *A,*B,*C,*D,*resul_intermedio,maxA,maxB,maxC,minA,minB,minC,avgA,avgB,avgC;
 int cons,aux,i,j,k,check=1;
 double timetick;

 //Controla los argumentos al programa
 if (argc != 3){
   printf("\n Faltan argumentos:: N dimension de la matriz, T cantidad de threads \n");
   return 0;
 }
 int N=atoi(argv[1]);
 int numThreads=atoi(argv[2]);
 omp_set_num_threads(numThreads);

 //Reservo memoria
 A=(double*)malloc(sizeof(double)*N*N);
 B=(double*)malloc(sizeof(double)*N*N);
 C=(double*)malloc(sizeof(double)*N*N);
 D=(double*)malloc(sizeof(double)*N*N);
 resul_intermedio=(double*)malloc(sizeof(double)*N*N);

 #pragma omp parallel private(i,j,k)
 { 

 //seteo las vmatrices, A con numeros random, inter,D, B y C en 0 para luego setear la diagonal con 1, todas las matrices se almacenaran orientado a filas, ya que 
 //las unicas que accederia por colomnas serian B y C pero como estas son la matris identidad es lo mismo accederlas por filas o columnas
 #pragma omp for
 for(i=0;i<(N*N);i++){
	A[i]=rand()%100;
    B[i]=0.0;
    C[i]=0.0;
    D[i]=0.0;
    resul_intermedio[i]=0.0;
 }
 //seteo las diagonales en 1 y tengo la matriz identidad
 #pragma omp for 
 for (i=0; i<N; i++){
	B[i*N+i]= 1.0;
    C[i*N+i]= 1.0;
 }
 //inicio el timer
 #pragma omp single 
 {
 timetick = dwalltime();
 }

 //Calculo los maximos, minimos y promedios
 #pragma omp for reduction(min:minA, minB, minC) 
    for(i=0;i<N*N;i++){
        if(A[i]<minA)
            minA = A[i];
        if(B[i]<minB)
            minB = B[i];
        if(C[i]<minC)
            minC = C[i];
    }
 #pragma omp for reduction(max:maxA, maxB, maxC) 
    for(i=0;i<N*N;i++){
        if(A[i]>maxA)
            maxA = A[i];
        if(B[i]>maxB)
            maxB = B[i];
        if(C[i]>maxC)
            maxC = C[i];
    }
 #pragma omp for reduction(+:avgA, avgB, avgC) 
    for(i=0;i<N*N;i++){
        avgA += A[i];
        avgB += B[i];
        avgC += C[i];
    } 
 //fin del for

 //calculo la constante multiplicativa de las matrices
 #pragma omp single 
 {
 avgA = avgA/numThreads; avgB = avgB/numThreads; avgC = avgC/numThreads;
 cons = ((maxA*maxB*maxC)-(minA*minB*minC))/(avgA*avgB*avgC);
 }   
 //calculo el producto de matrices

 //Realizo la primera parte de la multiplicacion resul_intermedio= A*B
  #pragma omp for 
  for(i=0;i<N;i++){
   for(j=0;j<N;j++){
    for(k=0;k<N;k++){
    resul_intermedio[i*N+j] = resul_intermedio[i*N+j] + A[i*N+k] * B[j*N+k]; 
    }
   }
  } 
 //Realizo la segunda multiplicacion  
  #pragma omp for  
  for(i=0;i<N;i++){
   for(j=0;j<N;j++){
    for(k=0;k<N;k++){
        D[i*N+j] = D[i*N+j] + resul_intermedio[i*N+k] * C[j*N+k]; 
    }
   }
  }
 //realizo la multiplicacion del resultado de A*B*C con la constante que calculamos previamente
 #pragma omp for 
 for (i=0;i<N*N;i++)
    D[i]= D[i]*cons;

 }//fin del parallel
 printf(" Tiempo en segundos de procesamiento %f \n", dwalltime() - timetick);
 //Verificacion del resultado
 printf("\n Realizando comprobacion ... \n" );
 for (i=0;i<N*N ;i++ )
 {
	 if ((A[i]*cons)!=D[i])
	 {
       check=0;
	 }
 } 
 if(check==1){
     printf(" Se realizo la multiplicacion correctamente \n");
 }else{
     printf(" Error en la multiplicacion \n");
 }

 free(A);
 free(B);
 free(C);
 free(D);
 free(resul_intermedio);
 return(0);
}


