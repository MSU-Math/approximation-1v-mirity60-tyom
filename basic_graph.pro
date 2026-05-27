QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -Wall -Wextra

HEADERS       = window.h \
                functions.h \
                lagrange.h \
                cubic_spline.h

SOURCES       = main.cpp \
                window.cpp \
                functions.cpp \
                lagrange.cpp \
                cubic_spline.cpp

QT += widgets
