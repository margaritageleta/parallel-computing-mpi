BINARIES=create_matrix reduce_mpi reduce_mpi_local reduce_mpi_local_withred reduce_mpi_openmp

all:$(BINARIES)

create_matrix:create_matrix.c
        gcc -o create_matrix create_matrix.c

reduce_mpi_local_withred:reduce_mpi_local_withred.c
        mpicc -o reduce_mpi_local_withred reduce_mpi_local_withred.c

reduce_mpi_openmp:reduce_mpi_openmp.c
        mpicc -qopenmp -o reduce_mpi_openmp reduce_mpi_openmp.c

reduce_mpi:reduce_mpi.c
        mpicc -o reduce_mpi reduce_mpi.c

reduce_mpi_local:reduce_mpi_local.c
        mpicc -o reduce_mpi_local reduce_mpi_local.c

clean:
        rm -rf $(BINARIES)

