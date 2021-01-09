#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

typedef struct {
    int **curGeneration;
    int **nextGeneration;
    int II, FI, IJ, FJ, N;
} threadArg;

int getNeighboorQuantity(int **generation, int i, int j, int N) {
    int north = (i - 1) >= 0 ? i - 1 : N - 1;
    int west = (j - 1) >= 0 ? j - 1 : N - 1;
    int east = (j + 1) < N ? j + 1 : 0;
    int south = (i + 1) < N ? i + 1 : 0;

    return generation[north][west] + generation[north][j] + generation[north][east] + generation[i][west] +
           generation[i][east] + generation[south][west] + generation[south][j] + generation[south][east];
}

void step(int **curGeneration, int **nextGeneration, int N) {
    int i, j;

    #pragma omp parallel private(i, j)
    #pragma omp for
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            int qtd = getNeighboorQuantity(curGeneration, i, j, N);

            int HP = curGeneration[i][j] ? (qtd == 2 || qtd == 3) : (qtd == 3);
            nextGeneration[i][j] = HP;
        }
    }
}

void printMatrix(int **m, int N) {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
}

void main() {
    int totalWorkers = 8;
    omp_set_num_threads(totalWorkers);
    int N = 2048;
    int totalGenerations = 2000;

    struct timeval inicio, final;
    float tmili = 0;

    int **curGeneration = (int**) malloc(N * sizeof(int*));
    int **nextGeneration = (int**) malloc(N * sizeof(int*));

    int i, j;
    srand(1985);
    for (i = 0; i < N; i++) {
        curGeneration[i] = (int*) malloc(N * sizeof(int));
        nextGeneration[i] = (int*) malloc(N * sizeof(int));
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            curGeneration[i][j] = rand() % 2;
        }
    }

    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            sum = sum + curGeneration[i][j];
        }
    }
    printf("Geracao 0: %d\n", sum);

    gettimeofday(&inicio, NULL);
	int k;
    for (k = 0; k < totalGenerations; k++) {

        step(curGeneration, nextGeneration, N);

        int **aux = curGeneration;
        curGeneration = nextGeneration;
        nextGeneration = aux;

    	// printf("Geracao %d: %d\n", k+1, sum);
    }

    sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            sum = sum + curGeneration[i][j];
        }
    }
    printf("Geracao %d: %d\n", k+1, sum);
    gettimeofday(&final, NULL);
    tmili = (int) (1000 * (final.tv_sec - inicio.tv_sec) + (final.tv_usec - inicio.tv_usec) / 1000);

    printf("gettimeofday: %.2f\n", tmili);
}