#ifndef WINDOW_H
#define WINDOW_H
#include <QWidget>
class QPainter;
class Window : public QWidget
{

  private:
    double a;
    double b;
    int n;
    int func_id;
    int graph_mode;
    int scale_power;
    int perturbation;

    bool lagrange_ready;
    bool spline_ready;

    double *nodes;
    double *values;
    double *lagrange_coef;
    double *spline_coef;
    double *spline_work;

    int prepare();
    void clear_data();

    double curve_value(int curve_id, double x) const;
    void find_bounds(double left, double right, double *min_y,
                     double *max_y) const;
    void draw_curve(QPainter &painter, double left, double right,
                    int curve_id) const;

  public:
    Window(QWidget *parent);
    ~Window();
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    int parse_command_line(int argc, char *argv[]);

  public:
    void change_func();
    void change_graph_mode();
    void zoom_in();
    void zoom_out();
    void increase_n();
    void decrease_n();
    void increase_perturbation();
    void decrease_perturbation();

  protected:
    void paintEvent(QPaintEvent *event);
};

#endif
