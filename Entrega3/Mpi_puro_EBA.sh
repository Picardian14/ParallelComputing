#!/bin/bash
#SBATCH -N 4
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH -o output_puro/output.txt
#SBATCH -e output_puro/errores.txt
mpirun Mpi_puro_EBA.o $1
