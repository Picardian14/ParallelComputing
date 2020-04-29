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

//Funcion que calcula el maximo de una matriz
double max (double * matriz, int N ){
    double vmax=-1;
    int i;
    for (i=0;i<N*N;i++)
        if (matriz[i] < vmax)
            vmax=matriz[i];

    return vmax;
}

//Funcion que calcula el minimo de una matriz
double min (double * matriz, int N ){
    double vmin=101;
    int i;
    for (i=0;i<N*N;i++)
        if (matriz[i] < vmin)
            vmin=matriz[i];
    
    return vmin;
}

//Funcion que calcula el promedio de una matriz
double avg (double * matriz, int N ){
    double avg=0;
    int i;
    for (i=0;i<N*N;i++)
        avg += matriz[i];

    avg = avg/(N*N);
    return avg;
}

int main(int argc,char*argv[]){

 double *A,*B,*C,*D,*resul_intermedio,maxA,maxB,maxC,minA,minB,minC,avgA,avgB,avgC;
 int cons,i,j,k,check=1;
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
 #pragma omp sections
 {
 #pragma omp section    
    maxA = max(A,N);
 #pragma omp section 
     maxB = max(B,N);
 #pragma omp section  
     maxC = max(C,N);
 #pragma omp section 
    minA = min(A,N);
 #pragma omp section 
     minB = min(B,N);
 #pragma omp section 
     minC = min(C,N);
 #pragma omp section 
     avgA = avg(A,N);
 #pragma omp section  
     avgB = avg(B,N);
 #pragma omp section      
     avgC = avg(C,N);
 }//fin de sections

 //calculo la constante multiplicativa de las matrices
 #pragma omp single 
 {
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
 printf(" Tiempo en segundos de procesamiento de matriz %ix%i con %d hilos: %f\n",N,N,numThreads, dwalltime() - timetick);
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
