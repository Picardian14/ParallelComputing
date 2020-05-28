#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=1
#SBATCH -o /nethome/spusuario5/Entrega3/output_hibrido/salida.txt
#SBATCH -e /nethome/spusuario5/Entrega3/output_hibrido/errores.txt
export OMP_NUM_THREADS=8
mpirun --bind-to none Mpi_hibrido.o $1
