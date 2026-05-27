#include "window.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow main_window;
    Window *graph_area = new Window(&main_window);

    if (graph_area->parse_command_line(argc, argv) != 0) {
        delete graph_area;
        return -1;
    }

    main_window.setCentralWidget(graph_area);

    QMenu *menu = main_window.menuBar()->addMenu("&Controls");

    QAction *action = nullptr;

    action =
        menu->addAction("Change function", graph_area, &Window::change_func);
    action->setShortcut(QString("Ctrl+C"));

    action = menu->addAction("Change graph mode", graph_area,
                             &Window::change_graph_mode);
    action->setShortcut(QString("Ctrl+M"));

    action = menu->addAction("Zoom in", graph_area, &Window::zoom_in);
    action->setShortcut(QString("Ctrl+2"));

    action = menu->addAction("Zoom out", graph_area, &Window::zoom_out);
    action->setShortcut(QString("Ctrl+1"));

    action = menu->addAction("Increase n", graph_area, &Window::increase_n);
    action->setShortcut(QString("Ctrl+4"));

    action = menu->addAction("Decrease n", graph_area, &Window::decrease_n);
    action->setShortcut(QString("Ctrl+3"));

    action = menu->addAction("Increase perturbation", graph_area,
                             &Window::increase_perturbation);
    action->setShortcut(QString("Ctrl+6"));

    action = menu->addAction("Decrease perturbation", graph_area,
                             &Window::decrease_perturbation);
    action->setShortcut(QString("Ctrl+5"));

    menu->addSeparator();

    action = menu->addAction("Exit", &app, &QApplication::quit);
    action->setShortcut(QString("Ctrl+Q"));

    main_window.setWindowTitle("Graph");
    main_window.resize(graph_area->sizeHint());
    main_window.show();

    return app.exec();
}
