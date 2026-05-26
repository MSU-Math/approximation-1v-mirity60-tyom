#ifndef LAGRANGE_H
#define LAGRANGE_H

int lagrange_init(int n, double *x, double *f, double *coef);
double lagrange_compute(double x, double a, double b, int n, double *x_array, double *coef);

#endif
