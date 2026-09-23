#include <stdio.h>
#include <math.h>
#include <mpi.h>

// Define a function to be integrated
double my_function(double *x) {
    double result = 0.0;
    result = cos(*x);
    return result;
}

int main(int argc, char *argv[]) {
    int num_of_ranks;
    int mpi_rank;
    double x, x1, h;
    double a = -0.5 * M_PI;
    double b = 0.5 * M_PI;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    if (mpi_rank == 0) {
        // Define interval length
        if (a < b) {
            h = (b - a) / ((double) num_of_ranks);
        } else if (a > b) {
            h = (a - b) / ((double) num_of_ranks);
        }

        for (int i = 1; i < num_of_ranks; i++) {
            MPI_Send(&h, 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&h, 1, MPI_DOUBLE, 0, mpi_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Compute Integral
    x = a + h * (double) mpi_rank;
    x1 = x + h;
    double val = h * (my_function(&x) + my_function(&x1)) / 2.0;


    if (mpi_rank != 0) {
        MPI_Send(&val, 1, MPI_DOUBLE, 0, mpi_rank, MPI_COMM_WORLD);
    } else {
        double integral = val;
        for (int i = 1; i < num_of_ranks; i++) {
            MPI_Recv(&val, 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            integral += val;
        }
        printf("Approximate Integral: %.10E\n", integral);
    }


    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}
