#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <mpi.h>

#include "Grid.cpp"

#define SEED 1985


int gridCount(int PID, int** generation, int BLOCK_SIZE, int N, int NUM_PROCESSES) {
  // partial block count
  int count = gridCount_serial(generation, BLOCK_SIZE, N);

  MPI_Request REQ;
  if (PID = 0) {
    int externalCount;

    // sum counts from other processes
    int p;
    for (p = 1; p < NUM_PROCESSES; p++) {
      MPI_Recv(&externalCount, 1, MPI_INT, p, 10, MPI_COMM_WORLD, NULL);
      count += externalCount;
    }

  } else {
    MPI_Isend(&count, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &REQ);
  }
  
  return count;
}

int main(int argc, char *argv[]) {
  int N = 2048;
  int NUM_GENERATIONS = 2000; 
  // 2000 == 146951
  // 15   == 753437
  // 0 = 2096241

  // MPI stuff
  int NUM_PROCESSES;
  int PID;
  int NAME_SIZE;
  char COMPUTER_NAME[MPI_MAX_PROCESSOR_NAME];

  double start;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &NUM_PROCESSES);
  MPI_Comm_rank(MPI_COMM_WORLD, &PID);
  MPI_Get_processor_name(COMPUTER_NAME, &NAME_SIZE);

  
  MPI_Barrier(MPI_COMM_WORLD); 
  start = MPI_Wtime();

  srand(SEED);

  int BLOCK_SIZE = (N / NUM_PROCESSES) + (N % NUM_PROCESSES > PID);
  
  std::ofstream file;
  if (PID == 0) {
    printf("\n(mpi)\n\n");
    printf("PROCESSES          %d on %s\n", NUM_PROCESSES, COMPUTER_NAME);
    printf("NUM_GENERATIONS    %d\n", NUM_GENERATIONS);
    printf("N                  %d\n", N);
    printf("SEED               %d\n", SEED);
    printf("BOARD              %d/%d -> %d\n", N, NUM_PROCESSES, BLOCK_SIZE);
    printf("\n");

    file.open ("mpi_" + std::to_string(N) + "_" + std::to_string(NUM_GENERATIONS) +  "_" + std::to_string(NUM_PROCESSES) + "_" + COMPUTER_NAME + ".csv");
    
    file << "(mpi)\n\n";
    file << "PROCESSES          " << NUM_PROCESSES << " on " << COMPUTER_NAME << '\n';
    file << "NUM_GENERATIONS    " << NUM_GENERATIONS << '\n';
    file << "N                  " << N << '\n';
    file << "BOARD              " << N << "/" << NUM_PROCESSES << " -> " << BLOCK_SIZE << '\n';
    file << "\n";
    file << "generation" << "," << "cells count" << "," << "elapsed time (ms)" << "\n";

  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  
  // first and last rows are for messaging
  int **generation = (int**) malloc((1 + BLOCK_SIZE + 1) * sizeof(int*));
  int **next = (int**) malloc((1 + BLOCK_SIZE + 1) * sizeof(int*));

  int i, j;
  for (i = 0; i < (1 + BLOCK_SIZE + 1); i++) {
    generation[i] = (int*) malloc(N * sizeof(int));
    next[i] = (int*) malloc(N * sizeof(int));
  }
  
  // initialize grid
  // [ —, —, —, —, ...,   —  ]
  // [ 0, 1, 2, 3, ..., N - 1]
  // [ 0, 1, 2, 3, ..., N - 1]
  //  ...
  // [ 0, 1, 2, 3, ..., N - 1]
  // [ 0, 1, 2, 3, ..., N - 1]
  // [ —, —, —, —, ...,   —  ]
  // first/last rows are empty
  int i0 = PID * BLOCK_SIZE;
  int iN = i0 + BLOCK_SIZE;
  for (i = 0; i < N; i++) {
    int local_i = (i % BLOCK_SIZE) + 1;

    for (j = 0; j < N; j++) {
      if (i0 <= i && i < iN) {
        generation[local_i][j] = rand() % 2;
      } else {
        rand() % 2;
      }
    }
  }
  
  MPI_Request REQ;
	int k;
  for (k = 0; k < NUM_GENERATIONS; k++) {
    MPI_Barrier(MPI_COMM_WORLD); 
    double start = MPI_Wtime();

    int PREVIOUS_PID = (PID - 1 + NUM_PROCESSES) % NUM_PROCESSES; // previous process
    int NEXT_PID = (PID + 1) % NUM_PROCESSES; // next process

    MPI_Isend(generation[1], N, MPI_INT, PREVIOUS_PID, 10, MPI_COMM_WORLD, &REQ); // send row 1 to previous
    MPI_Isend(generation[BLOCK_SIZE], N, MPI_INT, NEXT_PID, 10, MPI_COMM_WORLD, &REQ); // send row M to next

    MPI_Recv(generation[BLOCK_SIZE + 1], N, MPI_INT, NEXT_PID, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive row 1 from next, put at M + 1
    MPI_Recv(generation[0], N, MPI_INT, PREVIOUS_PID, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive row M from previous, put at 0

    // calculate next generation
    int i, j;
    for (i = 1; i <= BLOCK_SIZE; i++) {
      for (j = 0; j <= N; j++) {
        // cant move code below, its considerably faster executing here
        // than creating another function and calling it
        int neighborhood = gridNeighborhood(generation, BLOCK_SIZE, N, i, j);

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

    // advance generation
    int **tmp = generation;
    generation = next;
    next = tmp;
    
    // partial block count
    int localCount = gridCount_serial(generation, BLOCK_SIZE, N);

    // sub-process generation debug
    // printf("%d/%04d           %d\n", PID, k+1, localCount);
    
    // wait for all
    MPI_Barrier(MPI_COMM_WORLD);

    
    if (PID == 0) {
      int generationCount = localCount;
      int externalCount;

      // sum counts from other processes
      int p;
      for (p = 1; p < NUM_PROCESSES; p++) {
        MPI_Recv(&externalCount, 1, MPI_INT, p, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        generationCount += externalCount;
      }
      
      printf("-/%04d           %d        %.02fms\n", k+1, generationCount, (MPI_Wtime() - start) * 1000.0);
      file << k+1 << "," << generationCount << "," << ((MPI_Wtime() - start) * 1000.0) << "\n";
    } else {
      MPI_Isend(&localCount, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &REQ);
    }
  }
  

  // partial block count
  int count = gridCount_serial(generation, BLOCK_SIZE, N);

  
  // gridPrint(generation, BLOCK_SIZE, N, PID);

  if (PID == 0) {
    int externalCount;

    // sum counts from other processes
    int p;
    for (p = 1; p < NUM_PROCESSES; p++) {
      MPI_Recv(&externalCount, 1, MPI_INT, p, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      count += externalCount;
    }

    printf("\nCOUNT (%04d)     %d        %.02fms\n", NUM_GENERATIONS, count, (MPI_Wtime() - start) * 1000.0);

    file.close();
  } else {
    MPI_Isend(&count, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &REQ);
  }


  MPI_Finalize();
  

  return 0;
}