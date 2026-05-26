#include "window.h"

#include "cubic_spline.h"
#include "functions.h"
#include "lagrange.h"

#include <QPainter>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>

static const int MAX_LAGRANGE_N = 50;
static const int DRAW_POINTS = 1200;
static const double DEFAULT_A = -1.0;
static const double DEFAULT_B = 1.0;
static const int DEFAULT_N = 10;
static const int DEFAULT_FUNC_ID = 0;

static int read_double(const char *text, double *value)
{
    char *end = nullptr;

    *value = std::strtod(text, &end);

    if (end == text || *end != '\0') {
        return -1;
    }

    return 0;
}

static int read_int(const char *text, int *value)
{
    char *end = nullptr;
    const long result = std::strtol(text, &end, 10);

    if (end == text || *end != '\0') {
        return -1;
    }

    *value = static_cast<int>(result);
    return 0;
}

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    a = DEFAULT_A;
    b = DEFAULT_B;
    n = DEFAULT_N;
    func_id = DEFAULT_FUNC_ID;
    graph_mode = 2;
    scale_power = 0;
    perturbation = 0;

    lagrange_ready = false;
    spline_ready = false;

    nodes = nullptr;
    values = nullptr;
    lagrange_coef = nullptr;
    spline_coef = nullptr;
    spline_work = nullptr;

    prepare();
}

Window::~Window()
{
    clear_data();
}

QSize Window::minimumSizeHint() const
{
    return QSize(600, 400);
}

QSize Window::sizeHint() const
{
    return QSize(1000, 700);
}

int Window::parse_command_line(int argc, char *argv[])
{
    double new_a = 0.0;
    double new_b = 0.0;
    int new_n = 0;
    int new_func_id = 0;

    if (argc != 5) {
        std::printf("Usage: %s a b n k\n", argv[0]);
        return -1;
    }

    if (read_double(argv[1], &new_a) != 0 || read_double(argv[2], &new_b) != 0 ||
        read_int(argv[3], &new_n) != 0 || read_int(argv[4], &new_func_id) != 0) {
        std::printf("Incorrect command line arguments\n");
        return -1;
    }

    if (new_a == new_b || new_n < 2 || new_func_id < 0 || new_func_id > 6) {
        std::printf("Incorrect values: need a != b, n >= 2, 0 <= k <= 6\n");
        return -1;
    }

    if (new_a > new_b) {
        std::swap(new_a, new_b);
    }

    a = new_a;
    b = new_b;
    n = new_n;
    func_id = new_func_id;
    graph_mode = 2;
    scale_power = 0;
    perturbation = 0;

    return prepare();
}

void Window::clear_data()
{
    delete[] nodes;
    delete[] values;
    delete[] lagrange_coef;
    delete[] spline_coef;
    delete[] spline_work;

    nodes = nullptr;
    values = nullptr;
    lagrange_coef = nullptr;
    spline_coef = nullptr;
    spline_work = nullptr;
}

int Window::prepare()
{
    clear_data();

    lagrange_ready = false;
    spline_ready = false;

    if (n < 2 || a == b) {
        return -1;
    }

    try {
        nodes = new double[n];
        values = new double[n];
        spline_coef = new double[4 * (n - 1)];
        spline_work = new double[5 * n];

        if (n <= MAX_LAGRANGE_N) {
            lagrange_coef = new double[2 * n];
        }
    } catch (const std::bad_alloc &) {
        clear_data();
        return -2;
    }

    const double step = (b - a) / (n - 1);
    const double max_value = max_abs_function(a, b, func_id);

    for (int i = 0; i < n; ++i) {
        nodes[i] = a + step * i;
        values[i] = function_value(nodes[i], func_id);
    }

    values[n / 2] += perturbation * 0.1 * max_value;

    if (lagrange_coef != nullptr && lagrange_init(n, nodes, values, lagrange_coef) == 0) {
        lagrange_ready = true;
    }

    if (cubic_spline_init(n, nodes, values, spline_coef, spline_work) == 0) {
        spline_ready = true;
    }

    return 0;
}

double Window::curve_value(int curve_id, double x) const
{
    if (curve_id == 0) {
        return function_value(x, func_id);
    }

    if (curve_id == 1 && lagrange_ready) {
        return lagrange_compute(x, a, b, n, nodes, lagrange_coef);
    }

    if (curve_id == 2 && spline_ready) {
        return cubic_spline_compute(x, a, b, n, nodes, spline_coef);
    }

    if (curve_id == 3 && lagrange_ready) {
        return lagrange_compute(x, a, b, n, nodes, lagrange_coef) -
               function_value(x, func_id);
    }

    if (curve_id == 4 && spline_ready) {
        return cubic_spline_compute(x, a, b, n, nodes, spline_coef) -
               function_value(x, func_id);
    }

    return 0.0;
}

void Window::find_bounds(double left, double right, double *min_y, double *max_y) const
{
    bool first = true;

    for (int i = 0; i <= DRAW_POINTS; ++i) {
        const double t = static_cast<double>(i) / DRAW_POINTS;
        const double x = left + (right - left) * t;

        int curves[3];
        int count = 0;

        if (graph_mode == 0) {
            curves[0] = 0;
            count = 1;

            if (lagrange_ready) {
                curves[count] = 1;
                ++count;
            }
        } else if (graph_mode == 1) {
            curves[0] = 0;
            count = 1;

            if (spline_ready) {
                curves[count] = 2;
                ++count;
            }
        } else if (graph_mode == 2) {
            curves[0] = 0;
            count = 1;

            if (lagrange_ready) {
                curves[count] = 1;
                ++count;
            }

            if (spline_ready) {
                curves[count] = 2;
                ++count;
            }
        } else {
            if (lagrange_ready) {
                curves[count] = 3;
                ++count;
            }

            if (spline_ready) {
                curves[count] = 4;
                ++count;
            }
        }

        for (int j = 0; j < count; ++j) {
            const double y = curve_value(curves[j], x);

            if (!std::isfinite(y)) {
                continue;
            }

            if (first) {
                *min_y = y;
                *max_y = y;
                first = false;
            } else {
                *min_y = std::min(*min_y, y);
                *max_y = std::max(*max_y, y);
            }
        }
    }

    if (first) {
        *min_y = -1.0;
        *max_y = 1.0;
    }

    if (std::fabs(*max_y - *min_y) < 1e-14) {
        *min_y -= 1.0;
        *max_y += 1.0;
    }
}

void Window::draw_curve(QPainter &painter, double left, double right, int curve_id) const
{
    QPointF previous;
    bool has_previous = false;

    for (int i = 0; i <= DRAW_POINTS; ++i) {
        const double t = static_cast<double>(i) / DRAW_POINTS;
        const double x = left + (right - left) * t;
        const double y = curve_value(curve_id, x);

        if (!std::isfinite(y)) {
            has_previous = false;
            continue;
        }

        const QPointF current(x, y);

        if (has_previous) {
            painter.drawLine(previous, current);
        }

        previous = current;
        has_previous = true;
    }
}

void Window::paintEvent(QPaintEvent *event)
{
    (void)event;

    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    const double center = 0.5 * (a + b);
    const double half_length = 0.5 * (b - a) / std::pow(2.0, scale_power);
    const double left = center - half_length;
    const double right = center + half_length;

    double min_y = -1.0;
    double max_y = 1.0;

    find_bounds(left, right, &min_y, &max_y);

    const double max_abs = std::max(std::fabs(min_y), std::fabs(max_y));

    std::printf("Max: %.16e\n", max_abs);

    painter.save();

    painter.translate(0.0, height());
    painter.scale(width() / (right - left), -height() / (max_y - min_y));
    painter.translate(-left, -min_y);

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (min_y <= 0.0 && max_y >= 0.0) {
        painter.setPen(QPen(Qt::gray, 0));
        painter.drawLine(QPointF(left, 0.0), QPointF(right, 0.0));
    }

    if (left <= 0.0 && right >= 0.0) {
        painter.setPen(QPen(Qt::gray, 0));
        painter.drawLine(QPointF(0.0, min_y), QPointF(0.0, max_y));
    }

    if (graph_mode == 0) {
        painter.setPen(QPen(Qt::black, 0));
        draw_curve(painter, left, right, 0);

        if (lagrange_ready) {
            painter.setPen(QPen(Qt::red, 0));
            draw_curve(painter, left, right, 1);
        }
    } else if (graph_mode == 1) {
        painter.setPen(QPen(Qt::black, 0));
        draw_curve(painter, left, right, 0);

        if (spline_ready) {
            painter.setPen(QPen(Qt::blue, 0));
            draw_curve(painter, left, right, 2);
        }
    } else if (graph_mode == 2) {
        painter.setPen(QPen(Qt::black, 0));
        draw_curve(painter, left, right, 0);

        if (lagrange_ready) {
            painter.setPen(QPen(Qt::red, 0));
            draw_curve(painter, left, right, 1);
        }

        if (spline_ready) {
            painter.setPen(QPen(Qt::blue, 0));
            draw_curve(painter, left, right, 2);
        }
    } else {
        if (lagrange_ready) {
            painter.setPen(QPen(Qt::red, 0));
            draw_curve(painter, left, right, 3);
        }

        if (spline_ready) {
            painter.setPen(QPen(Qt::blue, 0));
            draw_curve(painter, left, right, 4);
        }
    }

    painter.restore();

    painter.setPen(Qt::black);

    painter.drawText(10, 20, QString("k=%1 %2").arg(func_id).arg(function_text(func_id)));
    painter.drawText(10, 40,
                     QString("n=%1  scale=%2  perturbation=%3")
                         .arg(n)
                         .arg(scale_power)
                         .arg(perturbation));
    painter.drawText(10, 60,
                     QString("mode=%1  max=%2").arg(graph_mode).arg(max_abs, 0, 'e', 3));
    painter.drawText(10, 80, "black: f, red: Lagrange, blue: natural cubic spline");

    if (!lagrange_ready) {
        painter.drawText(10, 100, QString("Lagrange is disabled for n > %1").arg(MAX_LAGRANGE_N));
    }
}

void Window::change_func()
{
    func_id = (func_id + 1) % 7;
    perturbation = 0;
    prepare();
    update();
}

void Window::change_graph_mode()
{
    graph_mode = (graph_mode + 1) % 4;
    update();
}

void Window::zoom_in()
{
    ++scale_power;
    update();
}

void Window::zoom_out()
{
    --scale_power;
    update();
}

void Window::increase_n()
{
    n *= 2;
    prepare();
    update();
}

void Window::decrease_n()
{
    if (n > 2) {
        n /= 2;

        if (n < 2) {
            n = 2;
        }

        prepare();
        update();
    }
}

void Window::increase_perturbation()
{
    ++perturbation;
    prepare();
    update();
}

void Window::decrease_perturbation()
{
    --perturbation;
    prepare();
    update();
}
