#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <stddef.h>
#define N 100 // Taille de la matrice (modifiable)
int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank);//pour avoir des donnees deffirentes dans chaque processeur

    int  nbre_lign_proc = N / size;
    int debut_lign = rank * nbre_lign_proc;
    int fin_lign = (rank == size - 1) ? N : debut_lign +  nbre_lign_proc;

    double *D_local = malloc(N *  nbre_lign_proc * sizeof(double));//: chaque processus n’a que les lignes de D qui l’intéressent
    double *A = malloc(N*N*N*N * sizeof(double));
    double *F_local = calloc(N *  nbre_lign_proc, sizeof(double));// résultat local pour les lignes calculées
    double *F_total = NULL;

    if (rank == 0) {
        for (int i = 0; i < N*N*N*N; i++) A[i] = (double) rand() / RAND_MAX;
    }

    MPI_Bcast(A, N*N*N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < N *  nbre_lign_proc; i++) {
        D_local[i] = (double) rand() / RAND_MAX;
    }

    double debut = MPI_Wtime();

    for (int a = 0; a<  nbre_lign_proc; a++) {
        int i = debut_lign + a;
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                for (int l = 0; l < N; l++) {
                    int idx = ((i * N + j) * N + k) * N + l;
                    sum += D_local[a * N + k] * A[idx];
                }
            }
            F_local[a * N + j] = sum;
        }
    }

    if (rank == 0) F_total = calloc(N * N, sizeof(double));
    MPI_Gather(F_local, N *  nbre_lign_proc, MPI_DOUBLE, F_total, N *  nbre_lign_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double Fin = MPI_Wtime();

    if (rank == 0) {
        printf("Temps d'exécution (réplication partielle) : %f secondes\n", Fin- debut);
    }

    free(D_local); free(A); free(F_local);
    if (rank == 0) free(F_total);

    MPI_Finalize();
    return 0;
}
