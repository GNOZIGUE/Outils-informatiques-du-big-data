#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>
#include<time.h>
#define N 100
void init_matrix(double* mat, int size) {
    for (int i = 0; i < size; i++) {
        mat[i] = (double) rand() / RAND_MAX;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank); // Seed unique par processus

    double *D = malloc(N * N * sizeof(double));
    double *A = malloc(N * N * N * N * sizeof(double)); // Simplification en 1D
    double *F_local = calloc(N * N, sizeof(double));
    double *F_total = NULL;

    if (rank == 0) {
        init_matrix(D, N);
        for (int i = 0; i < N*N*N*N; i++) {
            A[i] = (double) rand() / RAND_MAX;
        }
    }

    MPI_Bcast(D, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(A, N*N*N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int rows_per_proc = N / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? N : start_row + rows_per_proc;

    double start_time = MPI_Wtime();

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                for (int l = 0; l < N; l++) {
                    int idx = ((i * N + j) * N + k) * N + l;
                    sum += D[k * N + l] * A[idx];
                }
            }
            F_local[i * N + j] = sum;
        }
    }

    if (rank == 0) F_total = calloc(N * N, sizeof(double));
    MPI_Reduce(F_local, F_total, N * N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Temps d'exécution (réplication totale) : %f secondes\n", end_time - start_time);
    }

    free(D); free(A); free(F_local);
    if (rank == 0) free(F_total);

    MPI_Finalize();
    return 0;
}
