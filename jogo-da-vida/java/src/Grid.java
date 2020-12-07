import java.lang.*;
import java.util.*;

public class Grid extends Thread {
    private int[][] generation;
    private int[][] next;
    private int size;
    private int NUM_THREADS;
    private int thread;

    public Grid(int[][] generation, int[][] next, int size, int thread, int NUM_THREADS) {
        this.generation = generation;
        this.next = next;
        this.size = size;
        this.thread = thread;
        this.NUM_THREADS = NUM_THREADS;
    }

    public void run() {
        this.step();
    }

    private void step() {
        int i, j;
        int start = this.thread * (this.size / this.NUM_THREADS),
                end = start + (this.size / this.NUM_THREADS) - 1 + (i == this.NUM_THREADS - 1 && (this.size % this.NUM_THREADS));

        for (i = this.II; i <= this.FI; i++) {
            for (j = IJ; j <= this.FJ; j++) {
                int qtd = this.getNeighboorQuantity(i, j);
                int HP = this.curGeneration[i][j] == 1 ? ((qtd == 2 || qtd == 3) ? 1 : 0) : ((qtd == 3) ? 1 : 0);

                this.nextGeneration[i][j] = HP;
            }
        }
    }

    private int neighborhood(int x, int y) {
        int TOP = (y - 1 + size) % size;
        int BOTTOM = (y + 1) % size;
        int LEFT = (x - 1 + size) % size;
        int RIGHT = (x + 1) % size;

        return  this.generation[LEFT][TOP]    + this.generation[x][TOP]    + this.generation[RIGHT][TOP] +
                this.generation[LEFT][y]      + 0                     + this.generation[RIGHT][y] +
                this.generation[LEFT][BOTTOM] + this.generation[x][BOTTOM] + this.generation[RIGHT][BOTTOM];
    }

    static int count(int[][] generation) {
        int count = 0;
        for (int x = 0; x < generation.length; x++) {
            for (int y = 0; y < generation[x].length; y++) {
                count = count + generation[x][y];
            }
        }
        return count;
    }

    static int[][] randomize(int size, int seed) {
        Random rnd = new Random(seed);
        int[][] generation = new int[size][size];

        int x, y;
        for(x = 0; x < size; x++) {
            for(y = 0; y < size; y++) {
                generation[x][y] = rnd.nextInt(2147483647) % 2;
            }
        }

        return generation;
    }
}