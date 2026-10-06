#if defined _MPI
#include <mpi.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

double dot_product(int size, double *x, double *y) {
    double result = 0.0;
    for (int i = 0; i < size; i++)
        result += x[i] * y[i];
    return result;
}

void get_colomn(int size, int colomn, double **matrix, double *result) {
    for (int i = 0; i < size; i++) {
        result[i] = matrix[i][colomn];
    }
}

int main(int argc, char *argv[]) {
    int mpi_rank = 0;
    int num_of_ranks = 1;
    int rows;
    int cols;
    double **matrix;
    double *vector;
    double *colomn;

#if defined _MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    double starting_time, ending_time, accuacy_time;
    accuacy_time = MPI_Wtick();
    if (mpi_rank == 0) {
        printf("MPI timings are computed with an accuracy of : %10.3e seconds\n", accuacy_time);
    }
    starting_time = MPI_Wtime();
#endif

    // Initialize matrix
    if (mpi_rank == 0) {
        rows = 5;
        cols = 5;

        matrix = (double **) calloc(rows, sizeof(double *));
        for (int i = 0; i < rows; i++) {
            matrix[i] = (double *) calloc(cols, sizeof(double));
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = (double) i * cols + j;
            }
        }
        vector = (double *) calloc(rows, sizeof(double));
        for (int i = 0; i < rows; ++i) {
            vector[i] = (double) i;
        }
    }

#if defined _MPI
    // Process/Rank 0 send the vector and colomn to all other ranks
    if (mpi_rank == 0) {
        assert(rows == num_of_ranks);

        colomn = (double *) calloc(rows, sizeof(double));

        for (int i = 1; i < num_of_ranks; i++) {
            get_colomn(rows, i, matrix, colomn);
            MPI_Send(&rows, 1, MPI_INT, i, i, MPI_COMM_WORLD);
            MPI_Send(&colomn[0], rows, MPI_DOUBLE, i, i + num_of_ranks, MPI_COMM_WORLD);
            MPI_Send(&vector[0], rows, MPI_DOUBLE, i, i + 2 * num_of_ranks, MPI_COMM_WORLD);
        }
    }

    // Every other process receives the rank
    if (mpi_rank != 0) {
        MPI_Recv(&rows, 1, MPI_INT, 0, mpi_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        assert(rows == num_of_ranks);

        colomn = (double *) calloc(rows, sizeof(double));
        vector = (double *) calloc(rows, sizeof(double));

        // printf("MPI rank %d Received from %d, the number of elements : %d\n", mpi_rank, 0, rows);
        MPI_Recv(&colomn[0], rows, MPI_DOUBLE, 0, mpi_rank + num_of_ranks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&vector[0], rows, MPI_DOUBLE, 0, mpi_rank + 2 * num_of_ranks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double res = dot_product(rows, vector, colomn);

    if (mpi_rank == 0) {
        printf("%4.2f ", res);
        for (int i = 1; i < num_of_ranks; i++) {
            MPI_Recv(&res, 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%4.2f ", res);
        }
        printf("\n");
    } else {
        MPI_Send(&res, 1, MPI_DOUBLE, 0, mpi_rank, MPI_COMM_WORLD);
    }

#else
    colomn = (double *) calloc(rows, sizeof(double));
    for (int i = 0; i < cols; i++) {
        get_colomn(rows, i, matrix, colomn);
        printf("%4.2f ", dot_product(rows, vector, colomn));
    }
#endif

    // Finalize the MPI environment
#if defined _MPI
    ending_time = MPI_Wtime();
    printf("MPI rank %d, Time taken to send and receive vector : %f seconds\n", mpi_rank, ending_time - starting_time);
    MPI_Finalize();
#endif

    return 0;
}
