import java.util.Random;

public class Main {
    public static void main(String args[]) throws InterruptedException {
        int N = 2048;
        int NUM_THREADS = 4;
        int NUM_GENERATIONS = 2000;
        int SEED = 1985;

        int[][] generation = Grid.randomize(N, SEED);
        int[][] next = new int[N][N];

        System.out.println("Geracao 0: " + Grid.count(generation));

        long calcTime = 0;

        long startTime = System.currentTimeMillis();
        int k;
        int total;
        for (k = 0; k < NUM_GENERATIONS; k++) {
            Grid[] workers = new Grid[NUM_THREADS];

            for (i = 0; i < NUM_THREADS; i++) {
                int II = i * (N / NUM_THREADS);
                int FI = II + (N / NUM_THREADS) - 1 + (((i == NUM_THREADS - 1) && (N % NUM_THREADS != 0)) ? 1 : 0);
                int IJ = 0;
                int FJ = N - 1;
                workers[i] = new Grid(generation, next, N);
                workers[i].start();
            }

            for (i = 0; i < NUM_THREADS; i++) {
                workers[i].join();
            }


            int [][]aux = generation;
            generation = next;
            next = aux;

            System.out.println("Geracao " + (k + 1) + ": " + alive(generation, N));
        }

        calcTime = System.currentTimeMillis() - startTime;
        System.out.println("currentTimeMillis: " + calcTime);
    }
}
