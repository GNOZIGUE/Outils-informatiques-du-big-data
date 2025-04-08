#include <stdio.h>
#include <string.h> 
#include <mpi.h>

#define n 15

typedef struct {
    char nom[n];
    char prenom[n];
    int age;
} MyStruct;

int main(int argc, char **argv) {
    int rank, blocklens[3];
    MPI_Aint indices[3];
    MPI_Datatype old_types[3], mystruct;
    MyStruct value;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Définition des paramètres pour MPI_Type_create_struct */
    blocklens[0] = 1;
    blocklens[1] = 1;
    blocklens[2] = 1;

    old_types[0] = MPI_STRING;
    old_types[1] = MPI_STRING;
    old_types[2] = MPI_INT;

    /* Calcul des déplacements (offsets) */
    MPI_Aint base_address;
    MPI_Get_address(&value, &base_address);
    MPI_Get_address(&value.nom, &indices[0]);
    MPI_Get_address(&value.prenom, &indices[1]);
    MPI_Get_address(&value.age, &indices[2]);

    indices[0] = indices[0] - base_address;
    indices[1] = indices[1] - base_address;
    indices[2] = indices[2] - base_address;

    /* Création du type de données structuré */
    MPI_Type_create_struct(3, blocklens, indices, old_types, &mystruct);
    MPI_Type_commit(&mystruct);

    do {
        if (rank == 0) {
            printf("Ce programme lit un nom, un prénom et un âge.\n");
            fflush(stdout);
            printf("Donner un nom, un prénom et un age: ");
            fflush(stdout);
            if (scanf("%s %s %d", &value.nom, &value.prenom, &value.age) != 3) {
                value.age = -1; // Pour sortir de la boucle si la saisie est invalide
            }
        }

        MPI_Bcast(&value, 1, mystruct, 0, MPI_COMM_WORLD);

        printf("Processus %d a recu %s, %s et %d\n", rank, value.nom, value.prenom, value.age);
        fflush(stdout);

    } while (value.age >= 0);

    MPI_Type_free(&mystruct);
    MPI_Finalize();

    return 0;
}
