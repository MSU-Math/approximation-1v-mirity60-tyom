#include "functions.h"

#include <cmath>

static double square(double x)
{
    return x * x;
}

double function_value(double x, int k)
{
    switch (k) {
    case 0:
        return 1.0;
    case 1:
        return x;
    case 2:
        return x * x;
    case 3:
        return x * x * x;
    case 4:
        return x * x * x * x;
    case 5:
        return std::exp(x);
    case 6:
        return 1.0 / (25.0 * square(x) + 1.0);
    default:
        return 0.0;
    }
}

QString function_text(int k)
{
    switch (k) {
    case 0:
        return "f(x)=1";
    case 1:
        return "f(x)=x";
    case 2:
        return "f(x)=x^2";
    case 3:
        return "f(x)=x^3";
    case 4:
        return "f(x)=x^4";
    case 5:
        return "f(x)=exp(x)";
    case 6:
        return "f(x)=1/(25*x^2+1)";
    default:
        return "unknown";
    }
}

double max_abs_function(double a, double b, int k)
{
    const int points = 10000;
    double result = 0.0;

    for (int i = 0; i <= points; ++i) {
        const double t = static_cast<double>(i) / points;
        const double x = a + (b - a) * t;
        const double value = std::fabs(function_value(x, k));

        if (value > result) {
            result = value;
        }
    }

    return result;
}
