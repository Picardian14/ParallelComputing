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
    double max=-1;
    int i,j;
    
    for (i=0;i<N;i++){
        for(j=0; j < N; j++){
            if (matriz[i*N+j] > max)           
                max=matriz[i*N+j];
            }
    }


    return max;
}

//Funcion que calcula el minimo de una matriz
double min (double * matriz, int N ){
    double min=101;
    int i,j;
    
    for (i=0;i<N;i++){
        for(j=0; j < N; j++){
            if (matriz[i*N+j] < min)           
                min=matriz[i*N+j];
            }
    }
    return min;
}

//Funcion que calcula el promedio de una matriz
double avg (double * matriz, int N ){
    double avg=0;
    int i, j;

    for (i=0;i<N;i++)
        for(j=0; j<N; j++)
            avg += matriz[i*N+j];

    avg = avg/(N*N);
    return avg;
}

int main(int argc,char*argv[]){

 double *A,*B,*C,*D,*resul_intermedio,maxA,maxB,maxC,minA,minB,minC,avgA,avgB,avgC, row, col;
 int cons,i,j,k,s,check=1;
 double timetick;

 //Controla los argumentos al programa
 if (argc != 2){
   printf("\n Error en el pasaje de parametros, solo se espera la dimension de la matris");
   return 0;
 }
 int N=atoi(argv[1]);
 

 //Reservo memoria
 A=(double*)malloc(sizeof(double)*N*N);
 B=(double*)malloc(sizeof(double)*N*N);
 C=(double*)malloc(sizeof(double)*N*N);
 D=(double*)malloc(sizeof(double)*N*N);
 resul_intermedio=(double*)malloc(sizeof(double)*N*N);

 //seteo las vmatrices, A con numeros random, inter,D, B y C en 0 para luego setear la diagonal con 1, todas las matrices se almacenaran orientado a filas, ya que 
 //las unicas que accederia por colomnas serian B y C pero como estas son la matris identidad es lo mismo accederlas por filas o columnas

 
for(int row=0; row < N; row++){
        for(int col=0 ; col < N; col++){
            A[N*row+col] = rand()%100;
            if (row == col){
                C[N*row+col] = 1.0;
                B[N*row+col] = 1.0;
            }else{                
                B[N*row+col] = 0;
                C[N*row+col] = 0;
            }
        }
    }
 /*
 for(i=0;i<(N*N);i++){
	A[i]=rand()%100;
    B[i]=0.0;
    C[i]=0.0;
    D[i]=0.0;
    resul_intermedio[i]=0.0;
 }
 //seteo las diagonales en 1 y tengo la matriz identidad
	for (i=0; i<N; i++){
		B[i*N+i]= 1.0;
        C[i*N+i]= 1.0;
	}*/
 //Calculo los maximos, minimos y promedios
 timetick = dwalltime();
 maxA = max(A,N);
 maxB = max(B,N);
 maxC = max(C,N);
 minA = min(A,N);
 minB = min(B,N);
 minC = min(C,N);
 avgA = avg(A,N);
 avgB = avg(B,N);
 avgC = avg(C,N);

 //calculo la constante multiplicativa de las matrices
 cons = ((maxA*maxB*maxC)-(minA*minB*minC))/(avgA*avgB*avgC);

 //calculo el producto de matrices

 //Realizo la primera parte de la multiplicacion resul_intermedio= A*B

  for(i=0;i<N;i++){
   for(j=0;j<N;j++){
    for(k=0;k<N;k++){
    resul_intermedio[i*N+j] = resul_intermedio[i*N+j] + A[i*N+k] * B[j*N+k]; 
    }
   }
  } 
 //Realizo la segunda multiplicacion    
  for(i=0;i<N;i++){
   for(j=0;j<N;j++){
    for(k=0;k<N;k++){
    D[i*N+j] = D[i*N+j] + resul_intermedio[i*N+k] * C[j*N+k]; 
    }
   }
  }
 //realizo la multiplicacion del resultado de A*B*C con la constante que calculamos previamente 
 for (i=0;i<N*N;i++)
    D[i]= D[i]*cons;

 printf(" Tiempo en segundos de procesamiento de matriz %ix%i : %f\n",N,N, dwalltime() - timetick);

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

}
