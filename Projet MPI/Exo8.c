#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>

#define N 10000000   // Taille du tableau 
const MAX_NUMBS = 100; // Valeur maximale des éléments du tableau

int main(int argc, char** argv) {
    int rank, size;
    int* tab = NULL;  // Stocké uniquement par le maître
    int* tab_esclave = NULL;    // Partie traitée par chaque esclave
    int taille_tab_esclave;
    int somme_esclave = 0;
    int somme_totale = 0;
    double temps_debut, temps_fin;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Amorcez le générateur de nombres aléatoires pour obtenir des résultats différents à chaque fois
    srand(time(NULL));
    
    // Le maître initialise le tableau contenant des nombres aléatoires de 0 à 100
    if (rank == 0) {
        tab = (int*)malloc(N * sizeof(int));
        int i;
        for (i = 0; i < N; i++) {
            tab[i] = (rand()% MAX_NUMBS+1);  //Nbre entre 0 et 100
        }
 
	temps_debut = MPI_Wtime(); //Permet de lancer le chrono 

        // Calcul de la taille des sous-tableaux
        taille_tab_esclave = N / (size - 1);
    }

    // Envoie de la taille des sous-tableaux à tous les esclaves (diffusion)
    MPI_Bcast(&taille_tab_esclave, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocation mémoire pour les sous-tableaux (esclaves)
    if (rank != 0) {
        tab_esclave = (int*)malloc(taille_tab_esclave * sizeof(int));
    }

    // Le maître envoie les données aux esclaves
    if (rank == 0) {
    	int i;
        for (i = 1; i < size; i++) {
            int debut = (i - 1) * taille_tab_esclave; //Le debut du tableau pour chaque esclave 
            // Envoie bloquant des sous tableaux à chaque esclave
            MPI_Send(&tab[debut], taille_tab_esclave, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // Réception des sous tableaux par chaque esclave
        MPI_Recv(tab_esclave, taille_tab_esclave, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Calcul de la somme par chaque esclaves
    if (rank != 0) {
        somme_esclave = 0;
        int i;
        for (i = 0; i < taille_tab_esclave; i++) {
            somme_esclave += tab_esclave[i];
        }
        printf("L'esclave %d a trouvé une somme de : %d\n", rank, somme_esclave);
    }

    // Les esclaves envoient leur résultat au maître
    if (rank != 0) {
        MPI_Send(&somme_esclave, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Le maître somme les résultats reçus
    if (rank == 0) {
        somme_totale = 0;
        int i;
        for (i = 1; i < size; i++) {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            somme_totale += temp;
        }

        // Gestion du reste (si N n'est pas divisible par (size-1))
        int reste = N % (size - 1);
        if (reste != 0) {
            int i;
            for (i = N - reste; i < N; i++) {
                somme_totale += tab[i];
            }
        }
	
	temps_fin = MPI_Wtime(); // Arrêt du chrono
        printf("\nLa somme totale est : %d\n", somme_totale);
	printf("\nLe temps d'exécution est de %f secondes\n", temps_fin - temps_debut);
    }

    // Nettoyer les tableaux
    if (rank == 0) {
        free(tab);
    } else {
        free(tab_esclave);
    }

    MPI_Finalize();
    return 0;
}