#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include "Libraries/arrays.h"

#define N_GRID_POINTSX 512 // Grid interior points along x
#define N_GRID_POINTSY 512 // Grid interior points along y
#define N_TEMP_STEPS 10000   // Number of time steps
#define D 1.0              // Thermal diffusion constant
#define L 1.0              // Length of the side of the plane

int main() {
    MPI_Init(NULL, NULL);
    int num_of_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);


    int ranks_dimensions = 2;
    // TODO fix dimensions
    int ranks_per_dims[2] = {ranks_dimensions, ranks_dimensions}; // 3x4 grid (12 processes)
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


    // Create a type to send piece of matrix
    int rows = N_GRID_POINTSY / ranks_dimensions + 2;
    int cols = N_GRID_POINTSX / ranks_dimensions + 2;

    MPI_Datatype MPI_SEND_SUBMAT, MPI_RECV_SUBMAT;
    MPI_Type_vector(rows, cols, N_GRID_POINTSX + 2, MPI_DOUBLE, &MPI_SEND_SUBMAT);
    MPI_Type_commit(&MPI_SEND_SUBMAT);

    MPI_Type_vector(rows, cols, cols, MPI_DOUBLE, &MPI_RECV_SUBMAT);
    MPI_Type_commit(&MPI_RECV_SUBMAT);

    MPI_Datatype MPI_SEND_ROW, MPI_RECV_ROW, MPI_SEND_COL, MPI_RECV_COL;

    MPI_Type_contiguous(cols, MPI_DOUBLE, &MPI_SEND_ROW);
    MPI_Type_commit(&MPI_SEND_ROW);

    MPI_Type_contiguous(cols, MPI_DOUBLE, &MPI_RECV_ROW);
    MPI_Type_commit(&MPI_RECV_ROW);

    MPI_Type_vector(rows, 1, cols, MPI_DOUBLE, &MPI_SEND_COL);
    MPI_Type_commit(&MPI_SEND_COL);

    MPI_Type_vector(rows, 1, cols, MPI_DOUBLE, &MPI_RECV_COL);
    MPI_Type_commit(&MPI_RECV_COL);

    // Send part of matrix to ranks

    double** matrix_piece = allocate_2d_packed_double(&rows, &cols);
    double** matrix_piece_new = allocate_2d_packed_double(&rows, &cols);

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
                u[i][j] = sin(M_PI * x) * sin(M_PI * y);
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

        for (int i = 0; i < ranks_dimensions; i++) {
            for (int j = 0; j < ranks_dimensions; j++) {
                int r_coords[2];
                MPI_Cart_coords(MPI_COMM_2D, i * ranks_dimensions + j, 2, r_coords);
                if (i * ranks_dimensions + j == 0)
                    continue;
                MPI_Send(&u[r_coords[0] * (rows - 2)][r_coords[1] * (cols - 2)], 1, MPI_SEND_SUBMAT,
                         i * ranks_dimensions + j, i * ranks_dimensions + j, MPI_COMM_WORLD);
            }
        }
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                matrix_piece[i][j] = u[coords[0] * (rows - 2) + i][coords[1] * (cols - 2) + j];
    }
    else {
        MPI_Recv(&matrix_piece[0][0], 1, MPI_RECV_SUBMAT, 0, mpi_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }


    for (int step = 1; step <= N_TEMP_STEPS; step++) {
        MPI_Sendrecv(&matrix_piece[1][0], 1, MPI_SEND_ROW, up, 0,
                     &matrix_piece[rows - 1][0], 1, MPI_RECV_ROW, down, 0,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[rows - 2][0], 1, MPI_SEND_ROW, down, 1,
                     &matrix_piece[0][0], 1, MPI_RECV_ROW, up, 1,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[0][1], 1, MPI_SEND_COL, left, 2,
                     &matrix_piece[0][cols - 1], 1, MPI_RECV_COL, right, 2,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&matrix_piece[0][cols - 2], 1, MPI_SEND_COL, right, 3,
                     &matrix_piece[0][0], 1, MPI_RECV_COL, left, 3,
                     MPI_COMM_2D, MPI_STATUS_IGNORE);

        // Update interior grid points
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                matrix_piece_new[i][j] = matrix_piece[i][j] +
                    rx * (matrix_piece[i + 1][j] - 2.0 * matrix_piece[i][j] + matrix_piece[i - 1][j]) +
                    ry * (matrix_piece[i][j + 1] - 2.0 * matrix_piece[i][j] + matrix_piece[i][j - 1]);
            }
        }

        double** temp = matrix_piece;
        matrix_piece = matrix_piece_new;
        matrix_piece_new = temp;
    }


    for (int i = 0; i < num_of_ranks; i++) {
        if (mpi_rank == i) {
            printf("%d %d\n", coords[0], coords[1]);
            for (int j = 0; j < rows; j++) {
                for (int k = 0; k < cols; k++) {
                    printf("%1.3f ", matrix_piece[j][k]);
                }
                printf("\n");
            }
            printf("\n");
            printf("\n");
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_2D);
    }




    MPI_Type_free(&MPI_SEND_SUBMAT);
    MPI_Type_free(&MPI_RECV_SUBMAT);


    MPI_Type_free(&MPI_SEND_ROW);
    MPI_Type_free(&MPI_RECV_ROW);

    MPI_Type_free(&MPI_SEND_COL);
    MPI_Type_free(&MPI_RECV_COL);

    MPI_Finalize();

    return 0;
}
