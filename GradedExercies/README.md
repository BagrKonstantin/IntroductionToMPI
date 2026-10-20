# HW1_IMPI: Distributed Matrix-Vector Multiplication with MPI

This project implements a parallelized matrix-matrix-vector multiplication and norm computation using **MPI** in C. It includes custom helper libraries for memory allocation and linear algebra operations.

---

## 📂 Directory Structure

Ensure your project workspace is organized as follows:

```text
.
├── HW1_IMPI.c          # Main source file
├── Libraries/          # Custom helper libraries directory
│   ├── arrays.h        # 1D/2D array allocation and random initialization routines
│   └── linearalgebra.h # BLAS-like operations (dgemv, ddot)
├── compilation.sh      # Compilation script for HPC clusters (AION)
├── script.sh           # Slurm job submission script for AION
├── AnalyzingScripts/   # Directory with scripts that were used for analyzing the data
└── Run/                # Output of the calculations done with the AION, .csv with the results of calculations and plot of that data
```

---

## ⚙️ Dependencies

To successfully compile and run this program, you need:

1. **MPI Library**: An MPI implementation (e.g., OpenMPI, Intel MPI, MPICH) providing the `mpicc` wrapper and `mpi.h`.
2. **C Standard Math Library**: The standard math library (`-lm`).
3. **Custom Libraries**: The `Libraries/` folder containing `arrays.h` and `linearalgebra.h` must reside in the same directory as `HW1_IMPI.c`.

---

## 🔨 Compilation Instructions

### 1. Local Compilation

To compile the code locally on a workstation or a node with an MPI wrapper installed, run the following command in your terminal:

```bash
mpicc -o HW1_IMPI.exe HW1_IMPI.c -lm
```

*Note: Ensure that your `mpicc` compiler wrapper is properly installed and exposed in your system's `PATH`.

### 2. Compilation on the AION

To compile the code on the AION cluster:

1. Log in and request an interactive session.
2. Run the provided compilation script:
```bash
./compilation.sh
```


---

## 🚀 Execution Instructions

### Running Locally

You can run the compiled executable using `mpirun` or `mpiexec`. For example, to run across 4 MPI processes:

```bash
mpirun -n 4 ./HW1_IMPI.exe
```

### Running on AION via Slurm

To submit your job to the AION scheduler using the provided Slurm script, customize your job name and submit it with:

```bash
sbatch -J name_of_my_job script.sh
```