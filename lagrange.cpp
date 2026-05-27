#include "lagrange.h"

#include <cmath>

int lagrange_init(int n, double *x, double *f, double *coef)
{
    if (n <= 0 || x == nullptr || f == nullptr || coef == nullptr) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        coef[i] = f[i];
    }

    for (int i = 0; i < n; ++i) {
        double denominator = 1.0;

        for (int j = 0; j < n; ++j) {
            if (i != j) {
                denominator *= x[i] - x[j];
            }
        }

        if (denominator == 0.0) {
            return -1;
        }

        coef[n + i] = 1.0 / denominator;
    }

    return 0;
}

double lagrange_compute(double x, double a, double b, int n, double *x_array,
                        double *coef)
{
    (void)a;
    (void)b;

    if (n <= 0 || x_array == nullptr || coef == nullptr) {
        return 0.0;
    }

    double numerator = 0.0;
    double denominator = 0.0;

    for (int i = 0; i < n; ++i) {
        const double diff = x - x_array[i];

        if (std::fabs(diff) < 1e-14) {
            return coef[i];
        }

        const double term = coef[n + i] / diff;
        numerator += term * coef[i];
        denominator += term;
    }

    return numerator / denominator;
}
