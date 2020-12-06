#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "Grid.h"

void gridInitialize(Grid* grid, int size) {
  int **generation = (int**) malloc(size * sizeof(int*));
  int **next = (int**) malloc(size * sizeof(int*));

  int i;
  for (i = 0; i < size; i++) {
    generation[i] = (int*) malloc(size * sizeof(int));
    next[i] = (int*) malloc(size * sizeof(int));
  }

  grid->size = size;
  grid->generation = generation;
  grid->next = next;
}

void gridRandomize(Grid* grid) {
  int i, j;
  for (i = 0; i < grid->size; i++) {
    for (j = 0; j < grid->size; j++) grid->generation[i][j] = rand() % 2;
  }
}

int gridCount(Grid grid) {
  int count = 0, i, j;
  for (i = 0; i < grid.size; i++) {
    for (j = 0; j < grid.size; j++) {
      count = count + grid.generation[i][j];
    }
  }
  return count;
}

void gridAdvanceGeneration(Grid* grid) {
  int **temp = grid->generation;
  grid->generation = grid->next;
  grid->next = temp;
}

int gridNeighborhood(int **generation, int size, int x, int y) {
  int TOP = (y - 1 + size) % size;
  int BOTTOM = (y + 1) % size;
  int LEFT = (x - 1 + size) % size;
  int RIGHT = (x + 1) % size;

  return  generation[LEFT][TOP]    + generation[x][TOP]    + generation[RIGHT][TOP] +
          generation[LEFT][y]      + 0                     + generation[RIGHT][y] +
          generation[LEFT][BOTTOM] + generation[x][BOTTOM] + generation[RIGHT][BOTTOM];
}

void gridPrint(int **generation, int size) {
  int i, j;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) printf("%d ", generation[i][j]);
    printf("\n");
  }
}