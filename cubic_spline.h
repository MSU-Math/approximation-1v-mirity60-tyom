#ifndef CUBIC_SPLINE_H
#define CUBIC_SPLINE_H

int cubic_spline_init(int n, double *x, double *f, double *coef, double *work);
double cubic_spline_compute(double x, double a, double b, int n, double *x_array, double *coef);

#endif
