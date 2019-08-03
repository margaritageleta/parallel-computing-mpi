
#!/bin/bash
#SBATCH --job-name=mpi_mat
#SBATCH --output=mpiout.out
#SBATCH --error=mpiout.err
#SBATCH --nodes=1
#SBATCH --ntasks=5
#SBATCH --qos=debug
mpirun ./pruebasmat2 easy.txt
#mpirun ./reduce_mpi 4096.txt
