#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cblas.h"
int main(void) {
  int i;
  double A[6] = {1.0, 2.0, 1.0, -3.0, 4.0, -1.0};
  double B[6] = {1.0, 2.0, 1.0, -3.0, 4.0, -1.0};
  double C[9] = {.5, .5, .5, .5, .5, .5, .5, .5, .5};
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,
              3, 3, 2, 1, A, 3, B, 3, 2, C, 3);
  for (i = 0; i < 9; i++)
    printf("%lf ", C[i]);
  printf("\\n");
  if (fabs(C[0]-11) > 1.e-5) abort();
  if (fabs(C[4]-21) > 1.e-5) abort();
  return 0;
}