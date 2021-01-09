#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string.h>

#include "Grid.cpp"

#define SEED 1985
#define NUM_THREADS 8

int SUM_MODE = 0;
std::string SUM_MODE_TEXT = "serial";

int gridCount(Grid grid) {
  if (SUM_MODE == 1) return gridCount_critical(grid);
  else if (SUM_MODE == 2) return gridCount_reduction(grid);

  return gridCount_serial(grid);
}

int main(int argc, char const *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--critical") > 0 -1) { SUM_MODE = 1; SUM_MODE_TEXT = "critical"; }
  if (argc > 1 && strcmp(argv[1], "--reduction") > 0 -1) { SUM_MODE = 2; SUM_MODE_TEXT = "reduction"; }

  int N = 2048;
  int NUM_GENERATIONS = 15; 
  // 2000 == 146951
  // 15   == 753437

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
  printf("SUM_MODE           %s\n", SUM_MODE_TEXT.c_str());
  printf("\n");

  printf("GEN 0              %d\n", gridCount(grid));

  
  std::ofstream file;
  file.open ("openmp_" + std::to_string(N) + "_" + std::to_string(NUM_GENERATIONS) + "_" + SUM_MODE_TEXT + "_" + std::to_string(NUM_THREADS) + ".csv");
   
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

    int i, j;
    
    int **generation = grid.generation;
    int **next = grid.next;
    int size = grid.size;

    // break in threads
    #pragma omp parallel private(i, j) shared(generation, next, size)
    #pragma omp for collapse(2)
    for (i = 0; i <= size - 1; i++) {
      for (j = 0; j <= size - 1; j++) {
        // cant move code below, its considerably faster executing here
        // than creating another function and calling it
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
    
    gettimeofday(&localEnd, NULL);
    

    // next generation
    gridAdvanceGeneration(&grid);
    
    struct timeval sumStart, sumEnd;
    gettimeofday(&sumStart, NULL);

    int localCount = gridCount(grid);

    gettimeofday(&sumEnd, NULL);

    float localElapsed = (int) (1000 * (localEnd.tv_sec - localStart.tv_sec) + (localEnd.tv_usec - localStart.tv_usec) / 1000);
    float sumElapsed = (int) (1000 * (sumEnd.tv_sec - sumStart.tv_sec) + (sumEnd.tv_usec - sumStart.tv_usec) / 1000);

    printf("GEN %04d           %d        %.0f        %.0f\n", k+1, localCount, localElapsed, sumElapsed);
    file << localCount << "," << localElapsed << "," << sumElapsed << "\n";
    // alive cells, time elapsed to fill grid, time elapsed to sum alive cells
  }

  printf("GEN %04d           %d\n", NUM_GENERATIONS-1, gridCount(grid));

  gettimeofday(&end, NULL);
  elapsed = (int) (1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000);

  printf("elapsed time     %.2f\n", elapsed);

  return 0;
}