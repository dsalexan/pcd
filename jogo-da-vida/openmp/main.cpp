#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <omp.h>
#include <iostream>

#include "Grid.cpp"

#define SEED 1985
#define NUM_THREADS 8

int main() {
  int N = 2048;
  int NUM_GENERATIONS = 2000;

  struct timeval start, end;
  float ellapsed = 0;

  Grid grid;

  omp_set_num_threads(NUM_THREADS);
  srand(SEED);
  // initialize and prepare grid
  gridInitialize(&grid, N);
  gridRandomize(&grid);

  printf("(openmp)\n\n");
  printf("NUM_THREADS        %d\n", NUM_THREADS);
  printf("NUM_GENERATIONS    %d\n", NUM_GENERATIONS);
  printf("N                  %d\n", N);
  printf("SEED               %d\n", SEED);
  printf("\n");

  printf("GEN 0              %d\n", gridCount(grid));
  gettimeofday(&start, NULL);

  for (int k = 0; k < NUM_GENERATIONS; k++) { // for each generation
    // breack in threads
    #pragma omp parallel
    {
      int thread = omp_get_thread_num();

      int i, j;
      int start = thread * (grid.size / NUM_THREADS),
          end = start + (grid.size / NUM_THREADS) - 1 + (i == NUM_THREADS - 1 && (grid.size % NUM_THREADS));

      for (i = start; i <= end; i++) {
        for (j = 0; j <= grid.size - 1; j++) {
          // cant move code below, its considerably faster executing here
          // than creating another function and calling it
          int neighborhood = gridNeighborhood(grid.generation, grid.size, i, j);

          int state = grid.generation[i][j];
          if (state == 1) { // ALIVE
            // if (neighborhood >= 2 && neighborhood <= 3) state = 1;
            if (neighborhood < 2) state = 0; // death by abandonment
            else if (neighborhood >= 4) state = 0; // death by superpopulation
          } else { // DEAD
            if (neighborhood == 3) state = 1; // revivify
          }            
          
          grid.next[i][j] = state;
        }
      }
    }

    // next generation
    gridAdvanceGeneration(&grid);
  }

  printf("GEN %d           %d\n", NUM_GENERATIONS-1, gridCount(grid));

  gettimeofday(&end, NULL);
  ellapsed = (int) (1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000);

  printf("ellapsed time: %.2f\n", ellapsed);

  return 0;
}