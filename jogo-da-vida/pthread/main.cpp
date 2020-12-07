#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "Grid.cpp"

#define SEED 1985
#define NUM_THREADS 8

typedef struct {
  int size;
  int **generation;
  int **nextGeneration;

  int thread;
} arguments;


void* threadSimulate(void* _arg) {
  arguments arg = *((arguments *)(_arg));
  int size = arg.size;
  int** generation = arg.generation;
  int** next = arg.nextGeneration;
  int thread = arg.thread;

  int i, j;
  int start = thread * (size / NUM_THREADS),
      end = start + (size / NUM_THREADS) - 1 + (thread == NUM_THREADS - 1 && (size % NUM_THREADS));

  for (i = start; i <= end; i++) {
    for (j = 0; j <= size - 1; j++) {
      int neighborhood = gridNeighborhood(generation, size, i, j);

      int state = generation[i][j];
      if (state == 1) { // ALIVE
        // if (neighborhood >= 2 && neighborhood <= 3) state = 1;
        if (neighborhood < 2) state = 0; // death by abandonment
        else if (neighborhood >= 4) state = 0; // death by superpopulation
      } else { // DEAD
        if (neighborhood == 3) state = 1; // revivify
      }

      next[i][j] = state;
    }
  }

  
  pthread_exit(NULL);
}

int main() {
  int N = 2048;
  int NUM_GENERATIONS = 2000;

  struct timeval start, end;
  float ellapsed = 0;

  Grid grid;

  srand(SEED);
  // initialize and prepare grid
  gridInitialize(&grid, N);
  gridRandomize(&grid);

  printf("(pthread)\n\n");
  printf("NUM_THREADS        %d\n", NUM_THREADS);
  printf("NUM_GENERATIONS    %d\n", NUM_GENERATIONS);
  printf("N                  %d\n", N);
  printf("SEED               %d\n", SEED);
  printf("\n");

  printf("GEN 0              %d\n", gridCount(grid));
  gettimeofday(&start, NULL);

	int k;
  for (k = 0; k < NUM_GENERATIONS; k++) { // for each generation
    int i;
    pthread_t workers[NUM_THREADS];
    arguments arg[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) { // break calculations in clusters
      arg[i].size = grid.size;
      arg[i].generation = grid.generation;
      arg[i].nextGeneration = grid.next;
      arg[i].thread = i;

      // execute cluster (i) in a thread
      pthread_create(&workers[i], NULL, threadSimulate, (void *) &arg[i]);
    }

    // wait for threads
    for (i = 0; i < NUM_THREADS; i++) { pthread_join(workers[i], NULL); }

    // next generation
    gridAdvanceGeneration(&grid);
  }

  printf("GEN %d           %d\n", k-1, gridCount(grid));

  gettimeofday(&end, NULL);
  ellapsed = (int) (1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000);

  printf("ellapsed time: %.2f\n", ellapsed);

  return 0;
}