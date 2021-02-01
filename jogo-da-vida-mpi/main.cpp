#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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

void step(int **curGeneration, int **nextGeneration, int PN, int N) {
    int i, j;

    for (i = 1; i < PN - 1; i++) {
        for (j = 0; j <= N; j++) {
            int qtd = getNeighboorQuantity(curGeneration, i, j, N);

            int HP = curGeneration[i][j] ? (qtd == 2 || qtd == 3) : (qtd == 3);
            nextGeneration[i][j] = HP;
        }
    }
}

void printMatrix(int **m, int PN, int N, int P) {
    int i, j;
    for (i = 1; i < PN - 1; i++) {
        printf("P%d ", P);
        for (j = 0; j < N; j++) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int processId;
    int noProcesses;
    printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
    int N = 2048;
    int totalGenerations = 10;
    int i, j;
    printf("N %d gen %d\n", N, totalGenerations);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    int PN = (N / noProcesses) + (N % noProcesses > processId) + 2;
    int **curGeneration = (int**) malloc(PN * sizeof(int*));
    int **nextGeneration = (int**) malloc(PN * sizeof(int*));

    printf("Processo %d PN %d\n", processId, PN);

    for (i = 0; i < PN; i++) {
        curGeneration[i] = (int*) malloc(N * sizeof(int));
        nextGeneration[i] = (int*) malloc(N * sizeof(int));
    }

    for (i = 1; i < PN-1; i++) {
        int linha = i - 1 + (processId * (N / noProcesses)) + ((N % noProcesses) > processId ? processId : (N % noProcesses));
        for (j = 0; j < N; j++) {
            curGeneration[i][j] = (linha % 2) && (j % 2);
        }
    }

    // printf("Matrix %d\n", processId);
    // printMatrix(curGeneration, PN, N, processId);

    MPI_Request request;
	int k;
    for (k = 0; k < totalGenerations; k++) {
        // printf("De %d para %d\n", processId, !processId ? noProcesses - 1 : (processId - 1) % noProcesses);
        MPI_Isend(curGeneration[1], N, MPI_INT, !processId ? noProcesses - 1 : (processId - 1) % noProcesses, 10, MPI_COMM_WORLD, &request);
        // printf("De %d para %d\n", processId, processId == noProcesses - 1 ? 0 : (processId + 1) % noProcesses);
        MPI_Isend(curGeneration[PN - 2], N, MPI_INT, processId == noProcesses - 1 ? 0 : (processId + 1) % noProcesses, 10, MPI_COMM_WORLD, &request);

        // printf("Para %d de %d\n", processId, !processId ? noProcesses - 1 : (processId - 1) % noProcesses);
        // printf("Para %d de %d\n", processId, processId == noProcesses - 1 ? 0 : (processId + 1) % noProcesses);
        MPI_Recv(curGeneration[PN - 1], N, MPI_INT, processId == noProcesses - 1 ? 0 : (processId + 1) % noProcesses, 10, MPI_COMM_WORLD, NULL);
        MPI_Recv(curGeneration[0], N, MPI_INT, !processId ? noProcesses - 1 : (processId - 1) % noProcesses, 10, MPI_COMM_WORLD, NULL);

        // printf("%d received %d %d %d %d\n", processId, curGeneration[0][0], curGeneration[0][1], curGeneration[0][2], curGeneration[0][3]);
        // printf("%d received %d %d %d %d\n", processId, curGeneration[PN - 1][0], curGeneration[PN - 1][1], curGeneration[PN - 1][2], curGeneration[PN - 1][3]);

        step(curGeneration, nextGeneration, PN, N);

        int **aux = curGeneration;
        curGeneration = nextGeneration;
        nextGeneration = aux;
    }

    int sum = 0;
    for (i = 1; i < PN - 1; i++) {
        for (j = 0; j < N; j++) {
            sum = sum + curGeneration[i][j];
        }
    }
    // printf("Parcial %d: %d\n", processId, sum);

    if (processId == 0) {
        int sumAux;
        
        for (i = 1; i < noProcesses; i++) {
            MPI_Recv(&sumAux, 1, MPI_INT, i, 10, MPI_COMM_WORLD, NULL);
            sum += sumAux;
        }

        // printf("Final: %d\n", sum);
    }
    else {
        MPI_Isend(&sum, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &request);
    }

    return 0;\
    
    MPI_Finalize();
}