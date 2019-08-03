#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#define MASTER 0
int file_size(char *name,uint type_size) 
{
        int fd, size,size_bytes; 
        fd=open(name,O_RDONLY); 
        if (fd<0)       return -1; 
        size_bytes=lseek(fd,0,SEEK_END); 
        size = size_bytes/type_size; 
        close(fd); 
        return size; 
}


// lee toda la matriz desde una posicion dada.
int read_from_pos(char *name, uint pos,uint num_elems,uint type_size,void *buff)
{
        int fd,ret=0,pending,ready=0;
        fd=open(name,O_RDONLY); 
        if (fd<0)  return -1; 
        ret=lseek(fd,(pos*type_size),SEEK_SET); /
        if (ret!=pos*type_size) return -1; 
        pending=num_elems*type_size; 
        ready=0; 
        while(pending>0){ 
                ret=read(fd,(char *)buff+ready,pending);
                if (ret<0) return -1;
                pending=pending-ret;
                ready=ready+ret;
        }
        close(fd);
        return 0;
}
int *data; // puntero a donde se encuentran los datos
int total_elems; // numero total de elementos
int type_size; // 
int main (int argc, char *argv[]){
        MPI_Status status; // para guardar info sobre operaciones de recepcion.
        int my_rank,my_size; // tamano del comunicador y rango del proceso que lo llama
        int rc = -1; 
        int chunk; 
        int i; 

        type_size = sizeof(int); // tamano de un entero (8 Bytes en sist 64-bit sys)
        if (argc != 2){ // si no le pasamos la matriz, nos dice como usarlo
                printf("usage: %s file_name\n",argv[0]); // ./programa fichero
                exit(1); // el proceso te dice adios
        }

        total_elems = file_size(argv[1],sizeof(int)); // calcula el # de elems totales de la matriz 
        if (total_elems<0){ // no puedes tener un numero negativo de elementos 
                printf("Invalid number of elements\n"); 
                MPI_Abort(MPI_COMM_WORLD, rc); // apagamos el entorno MPI
        }

        double t1, t2;
        MPI_Init(&argc,&argv); // inicializamos el entorno MPI
        t1 = MPI_Wtime();
        MPI_Comm_rank(MPI_COMM_WORLD,&my_rank); // determinamos el rango del proceso que llama en el comunicaor
        MPI_Comm_size(MPI_COMM_WORLD,&my_size); // determinamos el tamano del comunicador (no inicializado??)

       
        chunk=total_elems/my_size; // calculamos el tamano del trozo.
        if(my_rank == MASTER)
        {
                printf("Number of MPI processes: %d\n", my_size);
                printf("Chunk size: %d\n", chunk);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        int elems_left;
        if(my_rank == my_size - 1)
        {
                elems_left = total_elems-((my_size-1)*chunk);
                 data = (int *)malloc(elems_left*sizeof(int));
        } else {
                elems_left = chunk;
                data  = (int *)malloc(elems_left*sizeof(int)); // asigna el tamano en bytes del trozo
        }
        if (data == NULL){ // nos aseguramos que no tengamos cosas raras
                printf("Error in malloc\n");
                MPI_Abort(MPI_COMM_WORLD, rc); // goodbye
        }

        int start = 0;
        int end = elems_left;
        int reduction = 0;
        for (i = start; i < end; i++)
        {
                reduction += data[i]; 

        }
        /* Send results */
        if (my_rank == MASTER){ // si es el proceso root
                int local_red;
                for (i=1;i<my_size;i++){ // por cada proceso esclavo
                        // root recibe de cada esclavo lo que ha recibido
                        if (MPI_Recv(&local_red,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE)!=MPI_SUCCESS){
                                printf("Error receiving local_red from rank %d\n",i);
                                MPI_Abort(MPI_COMM_WORLD, rc);
                        }
                        reduction+=local_red;
                }
                printf("Total reduction %d\n",reduction); // miramos el num total de elems que han enviado los esclavos.
                t2 = MPI_Wtime();
             printf("Time total exec: %1.9f\n", t2-t1);
                printf("\n");
        }else{ // los esclavos envian al root que es lo que han recibido de el
                if (MPI_Send(&reduction,1,MPI_INT,0,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
                        printf("Error sending reduction from %d to master\n",my_rank);
                        MPI_Abort(MPI_COMM_WORLD, rc);
                }
        }

        MPI_Finalize();
}

                                                     







