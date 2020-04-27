
#include "mxmSections.h"
#include <stdio.h>
void * prod(int N, double *A,double *B,double *D){
    int i,j,k;
    printf("Haciendo prod\n");
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            D[i*N+j]=0;
            for(k=0;k<N;k++){
                D[i*N+j]= D[i*N+j] + A[i*N+k]*B[k+j*N];
            }
        }
    }
}


void * sum(int N, double *C,double *B,double *E){
    int i,j,k;
    printf("Haciendo sum\n");
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            E[i*N+j]=0;
            for(k=0;k<N;k++){
                E[i*N+j]= E[i*N+j] + C[i*N+k]*B[k+j*N];
            }
        }
    }
}