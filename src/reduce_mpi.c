#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#define MASTER 0
int file_size(char *name,uint type_size) // o sea, le pasas la matriz y el tipo de los elementos
{
        int fd, size,size_bytes; // inicializas el descriptor fd
        fd=open(name,O_RDONLY); // lees la matrix que te hayan pasado
        if (fd<0)       return -1; // la puedes leer solo si la has pasado
        size_bytes=lseek(fd,0,SEEK_END); // lseek devuelve la posición del puntero resultante
        // medida en bytes desde el principio del fichero, como que lo colocamos en el END
        // te devuelve todos los bytes de la matriz
        size = size_bytes/type_size; // te parte los Bytes totales entre el tamano de un entero
        // asi consigues el num de numeritos que hay en la matriz.
        close(fd); // cierras el descriptor
        printf("There are %d elements in the file\n",size); // dices cuantos elems hay.
        return size; // y te vas
}


// lee toda la matriz desde una posicion dada.
int read_from_pos(char *name, uint pos,uint num_elems,uint type_size,void *buff)
{
        int fd,ret=0,pending,ready=0;
        fd=open(name,O_RDONLY); // con el descriptor, empiezas a leer la matriz
        if (fd<0)  return -1; // la puedes leer solo si la has pasado
        ret=lseek(fd, (pos*type_size), SEEK_SET); // se coloca al principio
        // y se mueve tantos numeros como indica la posicion.
        if (ret!=pos*type_size) return -1; // comprueba si se ha movido el num de bytes correcto
        pending=num_elems*type_size; // tamano total de la matriz
        ready=0; // Bytes leidos 0 de momento 
        while(pending>0){ // mientras falta por leer
                ret=read(fd,(char *)buff+ready,pending);
                if (ret<0) return -1;
                pending=pending-ret;
                ready=ready+ret;
        }
        printf("Total number of elements read %u\n",ready/type_size);
        close(fd);
        return 0;
}
int *data; // puntero a donde se encuentran los datos
int total_elems; // numero total de elementos
int type_size; // 
int main (int argc, char *argv[]){
        MPI_Status status; // para guardar info sobre operaciones de recepcion.
        int my_rank,my_size; // tamano del comunicador y rango del proceso que lo llama
        int rc = -1; //
        int chunk; //
        int i; //

        type_size = sizeof(int); // tamano de un entero (8 Bytes en sist 64-bit sys)
        if (argc != 2){ // si no le pasamos la matriz, nos dice como usarlo
                printf("usage: %s file_name\n",argv[0]); // ./programa fichero
                exit(1); // el proceso te dice adios
        }

        // de hechi, si realmente le hemos pasado la matriz entonces...
        double t1, t2, t1master, t2master;
        MPI_Init(&argc,&argv); // inicializamos el entorno MPI
        t1 = MPI_Wtime();
        // MPI_COMM_WORLD comunicador qie incluye todos los procesos del programa
        MPI_Comm_rank(MPI_COMM_WORLD,&my_rank); // determinadmos el rango del proceso que llama en el comunicaor
        MPI_Comm_size(MPI_COMM_WORLD,&my_size); // determinamos el tamano del comunicador 

        if (my_rank == MASTER) // si es el proceso MASTER (rango 0) entonces
        {
                printf("Using %s as input\n",argv[1]); // te dice que utiliza matriz tal como input
                total_elems = file_size(argv[1],sizeof(int)); // calcula el # de elems totales de la matriz
                // continuamos
                int elems_process; // 
                if (total_elems<0){ // no puedes tener un numero negativo de elementos 
                        printf("Invalid number of elements\n"); // ups
                        MPI_Abort(MPI_COMM_WORLD, rc); // apagamos el entorno MPI
                }

                printf("There are %d elems in the matrix\n",total_elems); // decimos cuantos elems hay en la matriz
                data = (int *)malloc(total_elems*sizeof(int)); // asigna el tamano en bytes de la matriz
                if (data == NULL){ // nos aseguramos que no tengamos cosas raras
                        printf("Error in malloc\n");
                        MPI_Abort(MPI_COMM_WORLD, rc); // goodbye
                }
                // supongamos que todo ha ido bien
                rc = read_from_pos(argv[1], 0, total_elems, type_size, (void *)data); //
                // le pasas el fichero de la matriz, el indice de la posicion, el numero total de elems, 
                // el tipo de los elementos
                // y la matriz de any type (por eso void, poruqe podria ser de floats etc)
                if (rc<0){ // comprobamos que hayamos podido leer toda la matriz
                        printf("Error reading file\n");
                        MPI_Abort(MPI_COMM_WORLD, rc);
                }
                /* Send data */ // Ahora ha llegado la hora a enviar mensajes
                chunk=total_elems/my_size; // calculamos el tamano del mensaje
                // partimos el num total de elems entre el numero de los procesos que hayan en el 
                // entorno MPI
                printf("\n");
                printf("Number of MPI processes: %d\n", my_size);
                printf("Chunk size: %d\n", chunk);
                printf("\n");
                t1master = MPI_Wtime();
                for(i=1;i<my_size;i++){ // para cada proceso esclavo (por eso empezamos por 1!)
                        if (i==(my_size-1)){
                                elems_process=total_elems-((my_size-1)*chunk); // = el chunk que queda
                                // para enviar, el ultimo
                        }else{
                                elems_process=chunk; // un chunk
                        }
                        // enviamos el unico chunk de tipo int de root al proceso i a traves del comunicador
                        // world. Si ha salido un error abortamos el entorno MPI.
                        if (MPI_Send(&elems_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD)!=MPI_SUCCESS){
                                                                printf("Error sending chunk\n");
                                MPI_Abort(MPI_COMM_WORLD, rc);
                        }
                        // ponemos el puntero a chunk*proceso correspondiente en el buffer data y cogemos
                        // a continuacion tantos elementos como hay en un chunk por proceso.
                        if (MPI_Send(data+(chunk*i),elems_process,MPI_INT,i,0, MPI_COMM_WORLD)!=MPI_SUCCESS){
                                printf("Error sending data\n");
                                MPI_Abort(MPI_COMM_WORLD, rc);
                        }
                }
                t2master = MPI_Wtime();
                printf("Time master: %1.9f\n",t2master-t1master);

        }else{ // si eres un proceso esclavo
                // recibes un unico chunk de tipo int  de root (0) con tag 0 a traves del
                // comunicador world. Si ha habido error, abortamos.
                if (MPI_Recv(&chunk,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE)!=MPI_SUCCESS){
                        printf("Error receiving chunk size\n");
                        MPI_Abort(MPI_COMM_WORLD, rc);
                }
                // asignamos el tamano en Bytes del chunk
                data = (int *)malloc(chunk*sizeof(int));
                // guardamos en el buffer data el chunk enviado por root. 
                if (MPI_Recv(data,chunk,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE)!=MPI_SUCCESS){
                        printf("Error receiving data\n");
                        MPI_Abort(MPI_COMM_WORLD, rc);
                }

        }
        /* At this point, we have the data */ // Tenemos los datos...
        int start = 0;
        int end = chunk;
        int reduction = 0;
        for (i = start; i < end; i++){
                reduction += data[i]; // en reduction guardamos todos los datos de la matriz

        }
        //printf("Reduction for rank %d is %d\n",my_rank,reduction);
        /* Send results */
        if (my_rank == MASTER){ // si es el proceso root
                int local_red;
                for (i=1;i<my_size;i++){ // por cada proceso esclavo
                        // guarda en el buffer local red un elemento
                        if (MPI_Recv(&local_red,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE)!=MPI_SUCCESS){
                                printf("Error receiving local_red from rank %d\n",i);
                                MPI_Abort(MPI_COMM_WORLD, rc);
                        }
                        //printf("Reduction received from rank %d is %d\n",i,local_red);
                        reduction+=local_red;
                }
                printf("Total reduction %d\n",reduction);
                t2 = MPI_Wtime();
                printf("Time: %1.9f\n", t2-t1);
        }else{
                if (MPI_Send(&reduction,1,MPI_INT,0,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
                        printf("Error sending reduction from %d to master\n",my_rank);
                        MPI_Abort(MPI_COMM_WORLD, rc);
                }
        }

        MPI_Finalize();
}

