#include "cubic_spline.h"

#include <cmath>

static int find_interval(double x, int n, double *nodes)
{
    if (x <= nodes[0]) {
        return 0;
    }

    if (x >= nodes[n - 1]) {
        return n - 2;
    }

    int left = 0;
    int right = n - 1;

    while (right - left > 1) {
        const int middle = left + (right - left) / 2;

        if (nodes[middle] <= x) {
            left = middle;
        } else {
            right = middle;
        }
    }

    return left;
}

int cubic_spline_init(int n, double *x, double *f, double *coef, double *work)
{
    if (n < 2 || x == nullptr || f == nullptr || coef == nullptr || work == nullptr) {
        return -1;
    }

    if (n == 2) {
        const double h = x[1] - x[0];

        if (h <= 0.0) {
            return -1;
        }

        coef[0] = f[0];
        coef[1] = (f[1] - f[0]) / h;
        coef[2] = 0.0;
        coef[3] = 0.0;

        return 0;
    }

    double *lower = work;
    double *diag = work + n;
    double *upper = work + 2 * n;
    double *rhs = work + 3 * n;
    double *d = work + 4 * n;

    for (int i = 0; i < n; ++i) {
        lower[i] = 0.0;
        diag[i] = 0.0;
        upper[i] = 0.0;
        rhs[i] = 0.0;
        d[i] = 0.0;
    }

    const double h0 = x[1] - x[0];

    if (h0 <= 0.0) {
        return -1;
    }

    const double delta0 = (f[1] - f[0]) / h0;

    diag[0] = 2.0;
    upper[0] = 1.0;
    rhs[0] = 3.0 * delta0;

    for (int i = 1; i < n - 1; ++i) {
        const double h_left = x[i] - x[i - 1];
        const double h_right = x[i + 1] - x[i];

        if (h_left <= 0.0 || h_right <= 0.0) {
            return -1;
        }

        const double delta_left = (f[i] - f[i - 1]) / h_left;
        const double delta_right = (f[i + 1] - f[i]) / h_right;

        lower[i] = h_right;
        diag[i] = 2.0 * (h_left + h_right);
        upper[i] = h_left;
        rhs[i] = 3.0 * (h_right * delta_left + h_left * delta_right);
    }

    const double hn = x[n - 1] - x[n - 2];

    if (hn <= 0.0) {
        return -1;
    }

    const double deltan = (f[n - 1] - f[n - 2]) / hn;

    lower[n - 1] = 1.0;
    diag[n - 1] = 2.0;
    rhs[n - 1] = 3.0 * deltan;

    for (int i = 1; i < n; ++i) {
        const double multiplier = lower[i] / diag[i - 1];

        diag[i] -= multiplier * upper[i - 1];
        rhs[i] -= multiplier * rhs[i - 1];
    }

    d[n - 1] = rhs[n - 1] / diag[n - 1];

    for (int i = n - 2; i >= 0; --i) {
        d[i] = (rhs[i] - upper[i] * d[i + 1]) / diag[i];
    }

    for (int i = 0; i < n - 1; ++i) {
        const double h = x[i + 1] - x[i];
        const double delta = (f[i + 1] - f[i]) / h;

        coef[4 * i] = f[i];
        coef[4 * i + 1] = d[i];
        coef[4 * i + 2] = (delta - d[i]) / h;
        coef[4 * i + 3] = (d[i] + d[i + 1] - 2.0 * delta) / (h * h);
    }

    return 0;
}

double cubic_spline_compute(double x, double a, double b, int n, double *x_array, double *coef)
{
    (void)a;
    (void)b;

    if (n < 2 || x_array == nullptr || coef == nullptr) {
        return 0.0;
    }

    const int i = find_interval(x, n, x_array);
    const double dx_left = x - x_array[i];
    const double dx_right = x - x_array[i + 1];

    const double c0 = coef[4 * i];
    const double c1 = coef[4 * i + 1];
    const double c2 = coef[4 * i + 2];
    const double c3 = coef[4 * i + 3];

    return c0 + c1 * dx_left + c2 * dx_left * dx_left +
           c3 * dx_left * dx_left * dx_right;
}
