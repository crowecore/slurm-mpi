#!/bin/bash
#SBATCH --job-name=matrix_mult
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=1
#SBATCH --time=00:10:00
#SBATCH --output=matrix_mult_%j.out

mpirun -n 4 ./matrix_mult