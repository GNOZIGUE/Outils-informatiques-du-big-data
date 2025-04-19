#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define N 10000000       // Taille du tableau 
const MAX_VALUE = 100;   // Valeur maximale des éléments du tableau

int main(int argc, char **argv) {
    int rank, size;     
    int *tab = NULL;   // Tableau créé par le maître
    int *tab_esclave = NULL;    // Sous tableau affecté aux esclaves
    int taille_tab_esclave;     // Taille des sous tableau 
    int somme_esclave = 0;      // Somme des éléments pour chaque esclave
    int somme_totale = 0;       // Somme totale calculée par le maître
    double temps_debut, temps_fin;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Le maître initialise le tableau global
    if (rank == 0) {
        tab = (int *)malloc(N * sizeof(int));
        srand(time(NULL));
        int i;
        for (i = 0; i < N; i++) {
            tab[i] = rand() % MAX_VALUE + 1;  // generer des nombres aléatoires entre 0 et 100
        }
        temps_debut = MPI_Wtime();
    }

    // Calcul des tailles des sous tableaux et des décalages pour gérer les restes
    int *NbreEnvoi = (int *)malloc(size * sizeof(int));
    int *deplacement = (int *)malloc(size * sizeof(int));
    int reste = N % size;
    int sum = 0;
	
	int i;
    for (i = 0; i < size; i++) {
        NbreEnvoi[i] = N / size + (i < reste ? 1 : 0); // si i < reste qui est N%size, reste = 1 sinon reste = 0.
        deplacement[i] = sum;
        sum += NbreEnvoi[i];
    }
    taille_tab_esclave = NbreEnvoi[rank];

    // Allocation mémoire pour le sous-tableau 
    tab_esclave = (int *)malloc(taille_tab_esclave * sizeof(int));

    // Distribution avec MPI_Scatterv qui permet de gèrer les tailles inégales
    MPI_Scatterv(tab, NbreEnvoi, deplacement, MPI_INT, tab_esclave, taille_tab_esclave, MPI_INT, 0, MPI_COMM_WORLD);

    // Somme dans chaque sous tableau effectuée par chaque processeur
    // if (rank != 0) {
         somme_esclave = 0;
         ///int i;
    	 for (i = 0; i < taille_tab_esclave; i++) {
        	somme_esclave += tab_esclave[i];
    	}
    	printf("Le processeur %d a trouvé une somme de : %d\n", rank, somme_esclave);
     //}

    // Réduction avec MPI_Reduce qui utilise une optimisation similaire à celle en arbre
    MPI_Reduce(&somme_esclave, &somme_totale, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Affichage du résultat par le maître
    if (rank == 0) {
        temps_fin = MPI_Wtime();
        printf("\nLa somme totale est : %d\n", somme_totale);
        printf("\nLe temps d'exécution est de %f secondes\n", temps_fin - temps_debut);
    }

    // Nettoyer les tableaux
    if (rank == 0) free(tab);
    free(tab_esclave);
    free(NbreEnvoi);
    free(deplacement);

    MPI_Finalize();
    return 0;
}