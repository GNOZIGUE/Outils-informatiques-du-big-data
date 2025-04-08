#include <stdio.h>
#include <mpi.h>

typedef struct {
    char nom[100];
    char prenom[100];
    int age;
} Etudiant;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Etudiant personne;

    if (rank == 0) {
        printf("Entrez le nom, le prenom et l'age : ");
        fflush(stdout);  // Pour s'assurer que le message est bien affiché avant la saisie
        scanf("%s %s %d", personne.nom, personne.prenom, &personne.age);
    }

    // Diffusion des informations à tous les processus
    MPI_Bcast(&personne, sizeof(Etudiant), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Affichage des informations reçues
    printf("Processus %d a recu : Nom = %s, Prenom = %s, Age = %d\n", rank, personne.nom, personne.prenom, personne.age);

    MPI_Finalize();
    return 0;
}
