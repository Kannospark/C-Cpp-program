#include "cblas.h"
#include <stdio.h>

void main()
{
  int i=0;
  double A[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};         
  double B[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};  
  double C[9] = {.5,.5,.5,.5,.5,.5,.5,.5,.5}; 
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,3,3,2,1,A, 3, B, 3,2,C,3);

  for(i=0; i<9; i++)
    printf("%lf ", C[i]);
  printf("\n");
}

// void matrix_vector(){
//     float array[6] = { 1,2,3,4,5,6 };
//     float x[3] = { 1,2,3 };
//     int m = 2; 
//     int n = 3;
//     blasint alpha = 1;
//     blasint beta = 1;
//     blasint inc_y = 1;
//     float y[2] = {};
//     cblas_sgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, array, n, x, 1, beta, y, inc_y);
//     //cblas_sgemv(CblasColMajor, CblasTrans, n, m, alpha, array, n, x, 1, beta, y, inc_y);
//     //两个结果一致
// }
// //打印结果
// // 14,32


// 3 3 3
// 3 3 3
// 1 1 1
// 1 1 1

// 2 2
// 2 2
// 2 2

// =
// 18 18
// 18 18
// 6 6
// 6 6