/*
    For this algorithm we have to use 4 process and divide the range in 4 process, after that we will use a reduction to find the riemann summ
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <assert.h>
#include "mpi.h"
#include <ctype.h>

#define X0 4
#define XN 10
#define N 1000
//In this case we have to use f(x) = x^3, but the rieman algorithm uses any positive function

double functionRiemann(double x){
    double y = 0.d;
    y = x*x*x;
    return y;
}

int main(int argc, char **argv)
{
    int rank, size;
    double b =(XN - X0)*1.0d/(N);
    double sum = 0.0d;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double li = X0*1.0d + b*rank*N/size;

    double lf = X0*1.0d + b*(rank + 1)*N/size;
    
    for(double i = li; i<lf-0.001; i = i + b){
        sum += functionRiemann(i)*b;
    } 
    double riemann_sum = 0.0d;
    
    MPI_Reduce(&sum, &riemann_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0){
        printf("Interval b = %f\n",b);
        printf("The riemann sum of a function x³ is: %f", riemann_sum);
    }
    MPI_Finalize();
}