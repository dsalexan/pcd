#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "Grid.cpp"

#define SEED 1985
#define NUM_THREADS 8

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

void step(int **curGeneration, int **nextGeneration, int II, int FI, int IJ, int FJ, int N) {
    int i, j;

    for (i = II; i <= FI; i++) {
        for (j = IJ; j <= FJ; j++) {
            // int qtd = getNeighboorQuantity(curGeneration, i, j, N);
            int qtd = gridNeighborhood(curGeneration, N, i, j);

            int HP = curGeneration[i][j] ? (qtd == 2 || qtd == 3) : (qtd == 3);
            nextGeneration[i][j] = HP;
        }
    }
}

void *middleware(void *arg) {
    threadArg tArg = *((threadArg *)(arg));
    step(tArg.curGeneration, tArg.nextGeneration, tArg.II, tArg.FI, tArg.IJ, tArg.FJ, tArg.N);
    pthread_exit(NULL);
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

int main() {
    int N = 2048;
    int totalGenerations = 2000;

    struct timeval inicio, final;
    float tmili = 0;

    srand(SEED);

    int **curGeneration = (int**) malloc(N * sizeof(int*));
    int **nextGeneration = (int**) malloc(N * sizeof(int*));

    int i, j;
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
        pthread_t workers[NUM_THREADS];
        threadArg tArgs[NUM_THREADS];

        for (i = 0; i < NUM_THREADS; i++) {
            tArgs[i].curGeneration = curGeneration;
            tArgs[i].nextGeneration = nextGeneration;
            tArgs[i].II = i * (N / NUM_THREADS);
            tArgs[i].FI = tArgs[i].II + (N / NUM_THREADS) - 1 + (i == NUM_THREADS - 1 && (N % NUM_THREADS));
            tArgs[i].IJ = 0;
            tArgs[i].FJ = N - 1;
			tArgs[i].N = N;
            pthread_create(&workers[i], NULL, middleware, (void *) &tArgs[i]);
        }

        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(workers[i], NULL);
        }

        int **aux = curGeneration;
        curGeneration = nextGeneration;
        nextGeneration = aux;

		sum = 0;
		for (i = 0; i < N; i++) {
		    for (j = 0; j < N; j++) {
		        sum = sum + curGeneration[i][j];
		    }
		}

    	printf("Geracao %d: %d\n", k+1, sum);
    }
    gettimeofday(&final, NULL);
    tmili = (int) (1000 * (final.tv_sec - inicio.tv_sec) + (final.tv_usec - inicio.tv_usec) / 1000);

    printf("gettimeofday: %.2f\n", tmili);
}