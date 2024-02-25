import java.util.Scanner;
import java.util.Random;

public class Compare {
    public static void main(String[] args){
        int n = 100000;
        for(int i = 0; i < 1; i++)
        calculate_time(n);
        System.out.println();
    }

    public static void calculate_time(int n){
        Random r = new Random();
        int[] a = new int[n];
        int[] b = new int[n];
        int[] c = new int[n];
        for(int i = 0; i < n; i++){
            a[i] = r.nextInt(1 << 16);
            b[i] = r.nextInt(1 << 16);
        }
        long start = System.nanoTime();
        for(int i= 0; i < n; i++){
            c[i] = a[i] * b[i];
        }
        long finish = System.nanoTime();
        long time_difference = (finish - start)/1000;
//        System.out.printf("%d ",time_difference);
    }
}