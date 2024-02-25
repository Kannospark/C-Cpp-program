import java.util.Random;

public class gptcode {
    public static void main(String[] args) {
        final int N = 100000000;
        double[] a = new double[N];
        double[] b = new double[N];
        Random random = new Random();
        for (int i = 0; i < N; ++i) {
            a[i] = random.nextDouble();
            b[i] = random.nextDouble();
        }

        long start = System.currentTimeMillis();
        double sum = 0.0;
        for (int i = 0; i < N; ++i) {
            sum += a[i] * b[i];
        }
        long end = System.currentTimeMillis();
        long duration = end - start;

        System.out.println("Java dot product computation took " + duration + " milliseconds");
        System.out.println("Dot product result: " + sum);
    }
}
