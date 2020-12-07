#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <omp.h>
#include <iostream>
#include <fstream>

#include "Grid.cpp"

#define SEED 1985
#define NUM_THREADS 1

int main() {
  int N = 2048;
  int NUM_GENERATIONS = 2000;

  struct timeval start, end;
  float elapsed = 0;

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

  
  std::ofstream file;
  file.open ("openmp_" + std::to_string(N) + "_" + std::to_string(NUM_GENERATIONS) + "_" + std::to_string(NUM_THREADS) + ".csv");
   
  file << "(openmp)\n\n";
  file << "NUM_THREADS        " << NUM_THREADS << '\n';
  file << "NUM_GENERATIONS    " << NUM_GENERATIONS << '\n';
  file << "N                  " << N << '\n';
  file << "SEED               " << SEED << '\n';
  file << "\n";


  gettimeofday(&start, NULL);

  for (int k = 0; k < NUM_GENERATIONS; k++) { // for each generation
    struct timeval localStart, localEnd;
    gettimeofday(&localStart, NULL);

    // breack in threads
    #pragma omp parallel
    {
      int thread = omp_get_thread_num();

      int i, j;
      int start = thread * (grid.size / NUM_THREADS),
          end = start + (grid.size / NUM_THREADS) - 1 + (thread == NUM_THREADS - 1 && (grid.size % NUM_THREADS));

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
    
    int localCount = gridCount(grid);
    gettimeofday(&localEnd, NULL);
    float localElapsed = (int) (1000 * (localEnd.tv_sec - start.tv_sec) + (localEnd.tv_usec - start.tv_usec) / 1000);

    // printf("GEN %d           %d        %.0f\n", k+1, localCount, localElapsed);
    file << localCount << "," << localElapsed << "\n";
  }

  printf("GEN %d           %d\n", NUM_GENERATIONS-1, gridCount(grid));

  gettimeofday(&end, NULL);
  elapsed = (int) (1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000);

  printf("elapsed time     %.2f\n", elapsed);

  return 0;
}