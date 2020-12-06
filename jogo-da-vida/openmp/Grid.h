typedef struct {
  int **generation;
  int **next;
  int size;
} Grid;

void gridInitialize(Grid* grid, int size);

void gridRandomize(Grid* grid);
int gridCount(Grid grid);
void gridAdvanceGeneration(Grid* grid);
int gridNeighborhood(int** generation, int size, int x, int y);

void gridPrint(int** generation, int size);