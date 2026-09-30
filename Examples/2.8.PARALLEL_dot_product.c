#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int min(int a, int b) {
    if (a < b) return a;
    return b;
}


int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int num_of_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    int elements;

    double *vector1;
    double *vector2;


    if (mpi_rank == 0) {
        int num_of_elements = 100;
        double *a;
        double *b;
        a = (double *) calloc(num_of_elements, sizeof(double));
        b = (double *) calloc(num_of_elements, sizeof(double));

        for (int i = 0; i < num_of_elements; i++) {
            a[i] = (double) i;
            b[i] = (double) i;
        }

        int h = num_of_elements / num_of_ranks;




        for (int i = 1; i < num_of_ranks; i++) {
            int from = h * i;
            int till = from + h;
            if (i == num_of_ranks - 1)
                till = num_of_elements;
            int to_send = till - from;

            vector1 = (double *) calloc(to_send, sizeof(double));
            vector2 = (double *) calloc(to_send, sizeof(double));


            for (int j = from; j < till; j++) {
                vector1[j - from] = a[j];
                vector2[j - from] = b[j];
            }

            MPI_Send(&to_send, 1, MPI_INT, i, i, MPI_COMM_WORLD);
            MPI_Send(&vector1[0], to_send, MPI_DOUBLE, i, i + num_of_ranks, MPI_COMM_WORLD);
            MPI_Send(&vector2[0], to_send, MPI_DOUBLE, i, i + num_of_ranks * 2, MPI_COMM_WORLD);
        }


        elements = h;
        vector1 = (double *) calloc(h, sizeof(double));
        vector2 = (double *) calloc(h, sizeof(double));
        for (int j = 0; j < h; j++) {
            vector1[j] = a[j];
            vector2[j] = b[j];
        }
    } else {
        MPI_Recv(&elements, 1, MPI_INT, 0, mpi_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        vector1 = (double *) calloc(elements, sizeof(double));
        vector2 = (double *) calloc(elements, sizeof(double));
        MPI_Recv(&vector1[0], elements, MPI_DOUBLE, 0, mpi_rank + num_of_ranks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&vector2[0], elements, MPI_DOUBLE, 0, mpi_rank + num_of_ranks * 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double c = 0.0;
    for (int i = 0; i < elements; i++) {
        c += vector1[i] * vector2[i];
    }
    if (mpi_rank == 0) {
        double recieved;
        for (int i = 1; i < num_of_ranks; i++) {
            MPI_Recv(&recieved, 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            c += recieved;
        }
        printf(" %4.2f ", c);
    } else {
        MPI_Send(&c, 1, MPI_DOUBLE, 0, mpi_rank, MPI_COMM_WORLD);
    }

    MPI_Finalize();


    return 0;
}
