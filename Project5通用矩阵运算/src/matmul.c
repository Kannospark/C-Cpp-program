#include <stdio.h>
#include <cblas.h>
#include <stdlib.h>
#include <sys/time.h>
#include "matmul.h"

// 开启OpenMP请解除注释以下代码
#define Use_openMP
// x86系统请解除注释以下代码
#define Use_SIMD_mavx2
// Arm系统请解除注释以下代码
// #define Use_arm_NEON

#ifdef Use_openMP
#include "omp.h"
#endif

#if defined Use_SIMD_mavx2
#include "immintrin.h"
#else
#ifdef Use_arm_NEON
#include "arm_neon.h"
#endif
#endif

void print_time_diff(struct timeval *begin, struct timeval *end)
{
    printf("%lf\n", (end->tv_sec - begin->tv_sec) + (double)(end->tv_usec - begin->tv_usec) / 1000000);
}

void array_random_init(double *array, size_t n)
{
    for (int i = 0; i < n; i++)
        array[i] = (double)rand() / RAND_MAX;
}

void array_zero_init(double *array, size_t n)
{
    for (int i = 0; i < n; i++)
        array[i] = 0.0;
}

void print_matrix(double *A, size_t m, size_t n)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            printf("%lf ", A[i * n + j]);
        printf("\n");
    }
}

// 最简单的直接乘法
void matrix_easy_mul(CBLAS_LAYOUT layout, CBLAS_TRANSPOSE TransA,
                     CBLAS_TRANSPOSE TransB, const int m, const int n,
                     const int k, const double alpha, const double *A,
                     const int lda, const double *B, const int ldb,
                     const double beta, double *C, const int ldc)
{
    if ((layout == CblasColMajor && TransA == CblasNoTrans) || (layout == CblasRowMajor && TransA == CblasTrans))
    {
        Trans(A, m, n);
    }
    if (TransB == CblasNoTrans)
    {
        Trans(B, k, n);
    }
    double *sum = (double *)malloc(sizeof(double) * m * n);
    for (int i = 0; i < m * n; i++)
        sum[i] = 0.0;
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int l = 0; l < k; l++)
            {
                sum[i * ldc + j] += A[i * lda + l] * B[j * ldb + l];
                // printf("sum[%d] = A[%d] * B[%d] = %lf\n",i * n + j,i * lda + l,j*ldb + l,sum[i * n + j]);
            }
        }
    }
    for (int i = 0; i < m * n; i++)
        C[i] = alpha * sum[i] + beta * C[i];
    if ((layout == CblasColMajor && TransA == CblasNoTrans) || (layout == CblasRowMajor && TransA == CblasTrans))
    {
        Trans(A, m, n);
    }
    if (TransB == CblasNoTrans)
    {
        Trans(B, k, n);
    }
}

// 矩阵乘法，默认行优先，无转置
void matrix_mul(CBLAS_LAYOUT layout, CBLAS_TRANSPOSE TransA,
                CBLAS_TRANSPOSE TransB, const int m, const int n,
                const int k, const double alpha, const double *A,
                const int lda, const double *B, const int ldb,
                const double beta, double *C, const int ldc)
{
    // 行排列直接矩阵乘法
    if ((layout == CblasRowMajor && TransA == CblasNoTrans) || (layout == CblasColMajor && TransA == CblasTrans))
    {
        if (TransB == CblasNoTrans)
        {
            // printf("method1\n");
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
            for (int i = 0; i < m; i++)
            {
                for (int l = 0; l < k; l++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        sum[ldc * i + j] += A[i * lda + l] * B[l * ldb + j];
                    }
                }
            }
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
        else
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    for (int l = 0; l < k; l++)
                    {
                        sum[i * ldc + j] += A[i * lda + l] * B[j * ldb + l];
                        // printf("sum[%d] = A[%d] * B[%d] = %lf\n",i * n + j,i * lda + l,j*ldb + l,sum[i * n + j]);
                    }
                }
            }
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
    }
    // 行排列转置后直接矩阵乘法
    else
    {
        // A矩阵列乘B矩阵行，得到n*m个矩阵求和
        if (TransB == CblasNoTrans)
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
            for (int l = 0; l < k; l++)
            {
                for (int i = 0; i < m; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        sum[i * ldc + j] += A[l * lda + i] * B[j + ldb * l];
                        // printf("sum[%d] += A[%d] * B[%d] = %f\n", i * n + j, l * m + i, j + n * l, sum[i * n + j]);
                    }
                }
            }
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
        else
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
            for (int j = 0; j < n; j++)
            {
                for (int l = 0; l < k; l++)
                {
                    for (int i = 0; i < m; i++)
                    {
                        sum[i + j * ldc] += A[l * lda + i] * B[j * ldb + l];
                        // printf("sum[%d] += A[%d] * B[%d]\n",i + j *m,l * m + i,j * k + l);
                    }
                }
            }
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    C[i] = alpha * sum[j * n + i] + beta * C[i * n + j];
                }
            }
            free(sum);
        }
    }
}

void matmul_SIMD_dotAdd(const double *A, const double *B, double *sum, size_t k)
{
#ifdef Use_SIMD_mavx2
    __m256d result;
    result = _mm256_setzero_pd();
    for (int i = 0; i < k - 3; i += 4)
    {
        result = _mm256_add_pd(result, _mm256_mul_pd(_mm256_loadu_pd(A + i), _mm256_loadu_pd(B + i)));
    }
    __m256d sum256_hadd = _mm256_hadd_pd(result, result);
    __m256d sum256_hadd_permute = _mm256_permute2f128_pd(sum256_hadd, sum256_hadd, 1);
    __m256d sum256_hadd_permute2 = _mm256_hadd_pd(sum256_hadd_permute, sum256_hadd_permute);
    __m128d sum128 = _mm256_extractf128_pd(sum256_hadd_permute2, 0);
    sum128 = _mm_add_pd(sum128, _mm256_extractf128_pd(sum256_hadd_permute2, 1));
    *sum += _mm_cvtsd_f64(sum128);
    for (int i = 0; i < k % 4; i++)
        *sum += A[k - 1 - i] * B[k - 1 - i];
    *sum = *sum / 2.0;
#endif
#ifdef Use_arm_NEON
    float64x2_t result;
    result = vdupq_n_f64(0.0);
    for (int i = 0; i < k - 3; i += 4)
    {
        result = vaddq_f64(result, vmulq_f64(vld1q_f64(A + i), vld1q_f64(B + i)));
    }
    // 将result中2个double相加得到一个double
    *sum += vpaddd_f64(result);
#endif
}

void matmul_SIMD_dotAdd_oneline(const double *A, const double *B, const double *sum, size_t m, size_t n)
{
#ifdef Use_SIMD_mavx2
    __m256d *result = (__m256d *)aligned_alloc(32, sizeof(__m256d) * (n / 4));
    for (int i = 0; i < (n / 4); i++)
    {
        result[i] = _mm256_setzero_pd();
    }
    for (int i = 0; i < m; i++)
    {
        for (int j = 0, l = 0; j < n - 3; j += 4, l++)
        {
            result[l] = _mm256_add_pd(result[l], _mm256_mul_pd(_mm256_broadcast_sd(A + i), _mm256_loadu_pd(B + j + i * n)));
        }
    }
    for (int l = 0, j = 0; l < n / 4; l++, j += 4)
    {
        _mm256_storeu_pd(sum + j, result[l]);
    }
    free(result);
#endif
#ifdef Use_arm_NEON
    float64x2_t *result = (float64x2_t *)aligned_alloc(32, sizeof(float64x2_t) * (n / 2));
    for (int i = 0; i < (n / 2); i++)
    {
        result[i] = vdupq_n_f64(0.0);
    }
    for (int i = 0; i < m; i++)
    {
        for (int j = 0, l = 0; j < n - 1; j += 2, l++)
        {
            result[l] = vaddq_f64(result[l], vmulq_f64(vdupq_n_f64(A[i]), vld1q_f64(B + j + i * n)));
        }
    }
    for (int l = 0, j = 0; l < n / 2; l++, j += 2)
    {
        vst1q_f64(sum + j, result[l]);
    }
    free(result);
#endif
}

void matmul_SIMD_addDotToMatrix(const double *A, const double *B, double *sum, size_t m, size_t n, size_t k)
{
#ifdef Use_SIMD_mavx2
    size_t moor = n / 4; //__m256d of one row
    __m256d *result = (__m256d *)aligned_alloc(32, sizeof(__m256d) * m * moor);
    for (int i = 0; i < m * moor; i++)
    {
        result[i] = _mm256_setzero_pd();
    }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (int l = 0; l < k; l++)
    {
        for (int i = 0; i < m; i++)
        {
            for (int j = 0, c = 0; j < n - 3; j += 4, c++)
            {
                result[i * moor + c] = _mm256_add_pd(result[i * moor + c], _mm256_mul_pd(_mm256_broadcast_sd(A + i + l * m), _mm256_loadu_pd(B + j + l * n)));
            }
        }
    }
    // new add
    for (int i = 0, l = 0; i < m * n; i += 4, l++)
    {
        _mm256_storeu_pd(sum + i, result[l]);
    }
    free(result);
#endif
#ifdef Use_arm_NEON
    size_t moor = n / 2;
    float64x2_t *result = (float64x2_t *)aligned_alloc(32, sizeof(float64x2_t) * m * moor);
    for (int i = 0; i < m * moor; i++)
    {
        result[i] = vdupq_n_f64(0.0);
    }
#ifdef Use_openMP
#pragma omp parallel for num_threads(4)
#endif
    for (int l = 0; l < k; l++)
    {
        for (int i = 0; i < m; i++)
        {
            for (int j = 0, c = 0; j < n - 1; j += 2, c++)
            {
                result[i * moor + c] = vaddq_f64(result[i * moor + c], vmulq_f64(vdupq_n_f64(A[i + l * m]), vld1q_f64(B + j + l * n)));
            }
        }
    }
// new add
#ifdef Use_openMP
#pragma omp parallel
#endif
    for (int i = 0, l = 0; i < m * n; i += 2, l++)
    {
        vst1q_f64(sum + i, result[l]);
    }
    free(result);
#endif
}

void matrix_SIMD_mul(CBLAS_LAYOUT layout, CBLAS_TRANSPOSE TransA,
                     CBLAS_TRANSPOSE TransB, const int m, const int n,
                     const int k, const double alpha, const double *A,
                     const int lda, const double *B, const int ldb,
                     const double beta, double *C, const int ldc)
{
    if ((layout == CblasRowMajor && TransA == CblasNoTrans) || (layout == CblasColMajor && TransA == CblasTrans))
    {
        if (TransB == CblasNoTrans) // 行乘行
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m; i++)
            {
                matmul_SIMD_dotAdd_oneline(A + lda * i, B, sum + n * i, m, n);
            }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
        else // 行乘列
        {    
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    matmul_SIMD_dotAdd(A + i * lda, B + j * ldb, sum + i * ldc + j, k);
                }
            }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
    }
    // 行排列转置后直接矩阵乘法
    else
    {
        // A矩阵列乘B矩阵行，得到n*m个矩阵求和
        if (TransB == CblasNoTrans) // 列乘行
        {
            // printf("method 3\n");
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
            matmul_SIMD_addDotToMatrix(A, B, sum, m, n, k);
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
        else // 列乘列
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int j = 0; j < n; j++)
            {
                matmul_SIMD_dotAdd_oneline(B + k * j, A, sum + j * m, n, m);
            }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    C[i * n + j] = alpha * sum[j * n + i] + beta * C[i * n + j];
                }
            }
            free(sum);
        }
    }
}

void matrix_SIMD_final_mul(CBLAS_LAYOUT layout, CBLAS_TRANSPOSE TransA,
                           CBLAS_TRANSPOSE TransB, const int m, const int n,
                           const int k, const double alpha, const double *A,
                           const int lda, const double *B, const int ldb,
                           const double beta, double *C, const int ldc)
{
    // 行排列直接矩阵乘法
    if ((layout == CblasRowMajor && TransA == CblasNoTrans) || (layout == CblasColMajor && TransA == CblasTrans))
    {
        if (TransB == CblasNoTrans) // 行乘行
        {
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m; i++)
            {
                matmul_SIMD_dotAdd_oneline(A + lda * i, B, sum + n * i, m, n);
            }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
        else // 行乘列
        {    
            double *sum = (double *)malloc(sizeof(double) * m * n);
            for (int i = 0; i < m * n; i++)
                sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    matmul_SIMD_dotAdd(A + i * lda, B + j * ldb, sum + i * ldc + j, k);
                }
            }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < m * n; i++)
                C[i] = alpha * sum[i] + beta * C[i];
            free(sum);
        }
    }
    // 行排列转置后直接矩阵乘法
    else
    {
        // printf("method 4\n");
        if(TransB == CblasNoTrans) Trans(B, k ,n);
        double *sum = (double *)malloc(sizeof(double) * m * n);
        for (int i = 0; i < m * n; i++)
            sum[i] = 0.0;
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (int j = 0; j < n; j++)
        {
            matmul_SIMD_dotAdd_oneline(B + k * j, A, sum + j * m, n, m);
        }
#ifdef Use_openMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                C[i * n + j] = alpha * sum[j * n + i] + beta * C[i * n + j];
                // printf("sum[%d]+C[%d] = %lf\n",j * n + i,i * n + j,sum[j * n + i]);
            }
        }
        free(sum);
        if(TransB == CblasNoTrans) Trans(B, k ,n);
    }
}

// 转置
double *Trans(double *A, int m, int n)
{
    double *temp = (double *)malloc(sizeof(double) * m * n);
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            temp[j * m + i] = A[i * n + j];
        }
    }
    for (int i = 0; i < m * n; i++)
    {
        A[i] = temp[i];
    }
    free(temp);
    return A;
}

// 计算结果的误差
void cal_error(const double *C, const double *C_copy, size_t l)
{
    double calculate_error = 0.0;
    double sum = 0.0;
    for (int i = 0; i < l; i++)
    {
        calculate_error += abs(C[i] - C_copy[i]);
        sum += abs(C_copy[i]);
    }
    printf("calculate error is: %lf%\n", calculate_error / sum * 100);
}