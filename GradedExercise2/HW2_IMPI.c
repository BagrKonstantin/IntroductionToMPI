#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include "Libraries/arrays.h"

#define N_GRID_POINTSX 512  // Grid interior points along x
#define N_GRID_POINTSY 512  // Grid interior points along y
#define N_TEMP_STEPS 10000  // Number of time steps
#define D 1.0               // Thermal diffusion constant
#define L 1.0               // Length of the side of the plane

int main() {
    MPI_Init(NULL, NULL);
    int num_of_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    // Measure initial system time
    double t0 = MPI_Wtime();

    int ranks_per_dims[2] = {0, 0};
    // Very cool function
    MPI_Dims_create(num_of_ranks, 2, ranks_per_dims);
    int periods[2] = {0, 0}; // Non-periodic boundaries
    int reorder = 1;
    MPI_Comm MPI_COMM_2D;

    // Create 2D Cartesian Communicator
    MPI_Cart_create(MPI_COMM_WORLD, 2, ranks_per_dims, periods, reorder, &MPI_COMM_2D);

    int rank, coords[2], left, right, up, down;
    MPI_Comm_rank(MPI_COMM_2D, &mpi_rank);
    // Find the coordinates of a give rank
    MPI_Cart_coords(MPI_COMM_2D, mpi_rank, 2, coords);

    // Find neighbors along dimension 1 (columns) 0 (rows)
    // MPI_Cart_shift(MPI_COMM_2D, 0, 1, &left, &right);
    // MPI_Cart_shift(MPI_COMM_2D, 1, 1, &down, &up);

    MPI_Cart_shift(MPI_COMM_2D, 0, 1, &up, &down);
    MPI_Cart_shift(MPI_COMM_2D, 1, 1, &left, &right);


    int rows = N_GRID_POINTSY / ranks_per_dims[0];
    int cols = N_GRID_POINTSX / ranks_per_dims[1];

    MPI_Datatype MPI_SEND_SUBMAT, MPI_RECV_SUBMAT;
    MPI_Type_vector(rows, cols, N_GRID_POINTSX + 2, MPI_DOUBLE, &MPI_SEND_SUBMAT);
    MPI_Type_commit(&MPI_SEND_SUBMAT);

    MPI_Type_vector(rows, cols, cols + 2, MPI_DOUBLE, &MPI_RECV_SUBMAT);
    MPI_Type_commit(&MPI_RECV_SUBMAT);

    MPI_Datatype MPI_SEND_ROW, MPI_RECV_ROW, MPI_SEND_COL, MPI_RECV_COL;

    MPI_Type_contiguous(cols, MPI_DOUBLE, &MPI_SEND_ROW);
    MPI_Type_commit(&MPI_SEND_ROW);

    MPI_Type_contiguous(cols, MPI_DOUBLE, &MPI_RECV_ROW);
    MPI_Type_commit(&MPI_RECV_ROW);

    MPI_Type_vector(rows, 1, cols + 2, MPI_DOUBLE, &MPI_SEND_COL);
    MPI_Type_commit(&MPI_SEND_COL);

    MPI_Type_vector(rows, 1, cols + 2, MPI_DOUBLE, &MPI_RECV_COL);
    MPI_Type_commit(&MPI_RECV_COL);

    MPI_Datatype MPI_GATHER_SUBMAT;
    MPI_Type_vector(rows, cols, N_GRID_POINTSX, MPI_DOUBLE, &MPI_GATHER_SUBMAT);
    MPI_Type_commit(&MPI_GATHER_SUBMAT);

    // Send part of matrix to ranks
    int alloc_rows = rows + 2, alloc_cols = cols + 2;
    double** matrix_piece = allocate_2d_packed_double(&alloc_rows, &alloc_cols);
    double** matrix_piece_new = allocate_2d_packed_double(&alloc_rows, &alloc_cols);

    for (int i = 0; i < rows + 2; i++)
        for (int j = 0; j < cols + 2; j++)
            matrix_piece[i][j] = matrix_piece_new[i][j] = 0.0;

    double dx = L / (N_GRID_POINTSX + 1);
    double dy = L / (N_GRID_POINTSY + 1);
    // Choose dt to satisfy stability criterion for explicit Euler: r = D * dt / dx^2 <= 0.5
    double dt = 0.124 / D * ((dx * dx) + (dy * dy));
    double rx = D * dt / (dx * dx);
    double ry = D * dt / (dy * dy);

    if (mpi_rank == 0) {
        int num_elementsx = N_GRID_POINTSX + 2;
        int num_elementsy = N_GRID_POINTSY + 2;
        double** u = allocate_2d_packed_double(&num_elementsx, &num_elementsy);

        // Initial Conditions: Create a hot square in the center
        for (int i = 1; i <= N_GRID_POINTSX; i++) {
            double x = i * dx;
            for (int j = 1; j <= N_GRID_POINTSY; j++) {
                double y = j * dy;
                u[i][j] = exp(-((x - L / 2) * (x - L / 2) + (y - L / 2) * (y - L / 2)) / 0.01);
            }
        }
        for (int i = 0; i <= N_GRID_POINTSY + 1; i++) {
            u[0][i] = 0.0;
            u[N_GRID_POINTSX + 1][i] = 0.0;
        }
        for (int i = 0; i <= N_GRID_POINTSX + 1; i++) {
            u[i][0] = 0.0;
            u[i][N_GRID_POINTSY + 1] = 0.0;
        }

        for (int i = 0; i < ranks_per_dims[0]; i++) {
            for (int j = 0; j < ranks_per_dims[1]; j++) {
                int r_coords[2];
                MPI_Cart_coords(MPI_COMM_2D, i * ranks_per_dims[1] + j, 2, r_coords);
                if (i * ranks_per_dims[1] + j == 0)
                    continue;
                MPI_Send(&u[r_coords[0] * rows + 1][r_coords[1] * cols + 1], 1, MPI_SEND_SUBMAT,
                         i * ranks_per_dims[1] + j, i * ranks_per_dims[1] + j, MPI_COMM_WORLD);
            }
        }
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                matrix_piece[i + 1][j + 1] = u[coords[0] * rows + i + 1][coords[1] * cols + j + 1];
    }
    else {
        MPI_Recv(&matrix_piece[1][1], 1, MPI_RECV_SUBMAT, 0, mpi_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    // Measure time before mathematical operations and communication
    double t1 = MPI_Wtime();

    double t_communication = 0.0;
    double t_computation = 0.0;

    for (int step = 1; step <= N_TEMP_STEPS; step++) {
        double communication_started = MPI_Wtime();
        MPI_Sendrecv(&matrix_piece[1][1], 1, MPI_SEND_ROW, up, 0,
                     &matrix_piece[rows + 1][1], 1, MPI_RECV_ROW, down, 0,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[rows][1], 1, MPI_SEND_ROW, down, 1,
                     &matrix_piece[0][1], 1, MPI_RECV_ROW, up, 1,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[1][1], 1, MPI_SEND_COL, left, 2,
                     &matrix_piece[1][cols + 1], 1, MPI_RECV_COL, right, 2,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[1][cols], 1, MPI_SEND_COL, right, 3,
                     &matrix_piece[1][0], 1, MPI_RECV_COL, left, 3,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);

        double computation_started = MPI_Wtime();
        // Update interior grid points
        for (int i = 1; i < rows + 1; i++) {
            for (int j = 1; j < cols + 1; j++) {
                matrix_piece_new[i][j] = matrix_piece[i][j] +
                    rx * (matrix_piece[i + 1][j] - 2.0 * matrix_piece[i][j] + matrix_piece[i - 1][j]) +
                    ry * (matrix_piece[i][j + 1] - 2.0 * matrix_piece[i][j] + matrix_piece[i][j - 1]);
            }
        }

        double** temp = matrix_piece;
        matrix_piece = matrix_piece_new;
        matrix_piece_new = temp;

        double end_iteration = MPI_Wtime();
        t_communication += computation_started - communication_started;
        t_computation += end_iteration - computation_started;
    }
    // Measure time after mathematical operations
    double t2 = MPI_Wtime();


    double total_temp_sum;
    double local_temp_sum = 0.0;
    for (int i = 1; i <= rows; i++)
        for (int j = 1; j <= cols; j++)
            local_temp_sum += matrix_piece[i][j];
    MPI_Reduce(&local_temp_sum, &total_temp_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_2D);
    if (mpi_rank == 0)
        printf("Average Temperature = %10.5f\n", total_temp_sum * dx * dy / (L * L));


    MPI_Type_free(&MPI_SEND_SUBMAT);
    MPI_Type_free(&MPI_RECV_SUBMAT);


    MPI_Type_free(&MPI_SEND_ROW);
    MPI_Type_free(&MPI_RECV_ROW);

    MPI_Type_free(&MPI_SEND_COL);
    MPI_Type_free(&MPI_RECV_COL);

    MPI_Type_free(&MPI_GATHER_SUBMAT);

    // Final time
    double t3 = MPI_Wtime();

    // Compute time differences and print them
    double dt0 = t1 - t0; // Time taken for initialization
    double dt1 = t2 - t1; // Time taken for calculation and communication
    double dt2 = t3 - t2; // Time taken for reduction
    double dt3 = t3 - t0; // Total time

    if (mpi_rank == 0) {
        MPI_Reduce(MPI_IN_PLACE, &dt0, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(MPI_IN_PLACE, &dt1, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(MPI_IN_PLACE, &t_computation, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(MPI_IN_PLACE, &t_communication, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(MPI_IN_PLACE, &dt2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(MPI_IN_PLACE, &dt3, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Reduce(&dt0, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&dt1, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&t_computation, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&t_communication, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&dt2, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&dt3, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    if (mpi_rank == 0) {
        printf("Average intiialization time per rank:            %f seconds\n", dt0 / ((double)num_of_ranks));
        printf("Average computation time per rank:               %f seconds\n", t_computation / ((double)num_of_ranks));
        printf("Average communication time per rank:             %f seconds\n", t_communication / ((double)num_of_ranks));
        printf("Average computation + calculation time per rank: %f seconds\n", dt1 / ((double)num_of_ranks));
        printf("Average reduction time per rank:                 %f seconds\n", dt2 / ((double)num_of_ranks));
        printf("Average Total time per rank:                     %f seconds\n", dt3 / ((double)num_of_ranks));
    }

    MPI_Finalize();

    return 0;
}
