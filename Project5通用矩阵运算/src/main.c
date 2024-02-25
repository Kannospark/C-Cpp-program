#include <stdio.h>
#include <cblas.h>
#include <stdlib.h>
#include <string.h>
#include "sys/time.h"
#include "matmul.h"

int main()
{
    int a = 4096;

    // while (a < 4000)
    // {
        struct timeval begin, end1, end2;
        size_t m, n, k, lda, ldb, ldc;
        double alpha, beta;
        a *= 2;
        printf("a=%d\n",a);
        m = a;
        n = a;
        k = a;
        lda = k;
        ldb = n;
        ldc = n;
        alpha = 1.0;
        beta = 1.0;
        double *A = (double *)malloc(sizeof(double) * m * k);
        double *B = (double *)malloc(sizeof(double) * k * n);
        double *C = (double *)malloc(sizeof(double) * m * n);
        double *C_copy = (double *)malloc(sizeof(double) * m * n);
        array_random_init(A, m * k);
        array_random_init(B, k * n);
        array_zero_init(C, m * n);
        array_zero_init(C_copy, m * n);
        // printf("matrix:A\n");
        // print_matrix(A, m, k);
        // printf("matrix:B\n");
        // print_matrix(B, k, n);
        gettimeofday(&begin, NULL);
        // Trans(B,n,k);
        matrix_SIMD_final_mul(CblasRowMajor, CblasTrans, CblasTrans, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
        // Trans(B,n,k);
        gettimeofday(&end1, NULL);
        cblas_dgemm(CblasRowMajor, CblasTrans, CblasTrans, m, n, k, alpha, A, lda, B, ldb, beta, C_copy, ldc);
        gettimeofday(&end2, NULL);
        // printf("matrix:C\n");
        // print_matrix(C, m, n);
        // printf("matrix:C_copy\n");
        // print_matrix(C_copy, m, n);
        print_time_diff(&begin, &end1);
        print_time_diff(&end1, &end2);
        cal_error(C, C_copy, a*a);
        free(A);
        free(B);
        free(C);
        free(C_copy);
    // }
    return 0;
}
