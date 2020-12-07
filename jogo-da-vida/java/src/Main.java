import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Main {
    public static String convertToCSV(String[] row) {
        return Stream.of(row)
                .collect(Collectors.joining(","));
    }

    public static void main(String args[]) throws InterruptedException, FileNotFoundException {
        int N = 2048;
        int NUM_THREADS = 8;
        int NUM_GENERATIONS = 2000;
        int SEED = 1985;

        int[][] generation = Grid.randomize(N, SEED);
        int[][] next = new int[N][N];

        System.out.println("(javathreads)");
        System.out.println("NUM_THREADS        " + NUM_THREADS);
        System.out.println("NUM_GENERATIONS    " + NUM_GENERATIONS);
        System.out.println("N                  " + N);
        System.out.println("SEED               " + SEED);
        System.out.println("\n");

        System.out.println("GEN 0              " + Grid.count(generation));

        long startTime = System.currentTimeMillis();
        List<String[]> times = new ArrayList<>();

        int k, i;
        for (k = 0; k < NUM_GENERATIONS; k++) {
            long localStart = System.currentTimeMillis();

            Grid[] workers = new Grid[NUM_THREADS];

            for (i = 0; i < NUM_THREADS; i++) {
                workers[i] = new Grid(generation, next, N, i, NUM_THREADS);
                workers[i].start();
            }

            for (i = 0; i < NUM_THREADS; i++) {
                workers[i].join();
            }

            int [][]aux = generation;
            generation = next;
            next = aux;

            int localCount = Grid.count(generation);
            long localElapsed = (System.currentTimeMillis() - localStart);
            times.add(new String[]{String.valueOf(localCount), String.valueOf(localElapsed)});
//            System.out.println("GEN " + k + "           " + localCount + "        " + localElapsed);
        }
        System.out.println("GEN " + k + "           " + Grid.count(generation));

        System.out.println("ellapsed time     " + (System.currentTimeMillis() - startTime));

        File csv = new File(N + "_" + NUM_GENERATIONS + "_" + NUM_THREADS + ".csv");
        try(PrintWriter writer = new PrintWriter((csv))) {
            times.stream()
                .map(Main::convertToCSV)
                .forEach(writer::println);
        }
    }
}
