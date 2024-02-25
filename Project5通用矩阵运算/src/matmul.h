#include <sys/time.h>
#include <stdio.h>
#include <cblas.h>

void matrix_easy_mul(CBLAS_LAYOUT, CBLAS_TRANSPOSE,
                CBLAS_TRANSPOSE, const int, const int,
                const int, const double, const double*,
                const int, const double*, const int,
                const double, double*, const int);

void matrix_mul(CBLAS_LAYOUT, CBLAS_TRANSPOSE,
                CBLAS_TRANSPOSE, const int, const int,
                const int, const double, const double*,
                const int, const double*, const int,
                const double, double*, const int);

void matrix_SIMD_mul(CBLAS_LAYOUT, CBLAS_TRANSPOSE,
                CBLAS_TRANSPOSE, const int, const int,
                const int, const double, const double*,
                const int, const double*, const int,
                const double, double*, const int);

void matrix_SIMD_final_mul(CBLAS_LAYOUT, CBLAS_TRANSPOSE,
                CBLAS_TRANSPOSE, const int, const int,
                const int, const double, const double*,
                const int, const double*, const int,
                const double, double*, const int);

void print_time_diff(struct timeval *, struct timeval *);
void print_matrix(double*, size_t, size_t);
void array_random_init(double*, size_t);
void array_zero_init(double*, size_t);
void matmul_SIMD_dotAdd(const double*, const double*, double*, size_t);
void matmul_SIMD_dotAdd_oneline(const double*, const double*, const double*, size_t, size_t);
double* Trans(double*, int, int);
void cal_error(const double*,const double*, size_t);

