#!/bin/bash
./create_matrix 1 32768000 giga
srun --nodes=1 --ntasks-per-node=6 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=2 --ntasks-per-node=12 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=3 --ntasks-per-node=18 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=4 --ntasks-per-node=24 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=5 --ntasks-per-node=30 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=6 --ntasks-per-node=36 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=7 --ntasks-per-node=42 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=8 --ntasks-per-node=48 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=9 --ntasks-per-node=54 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=10 --ntasks-per-node=60 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=11 --ntasks-per-node=66 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=12 --ntasks-per-node=72 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=13 --ntasks-per-node=78 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=14 --ntasks-per-node=84 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=15 --ntasks-per-node=90 --cpu-per-task=8 reduce_mpi_openmp giga
srun --nodes=16 --ntasks-per-node=96 --cpu-per-task=8 reduce_mpi_openmp giga
