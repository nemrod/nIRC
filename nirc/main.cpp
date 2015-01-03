#include <QApplication>

#include "nIRC.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    nIRC window;

    window.setWindowTitle(window.tr("nIRC 0.1"));
    window.resize(1000, 700);
    window.show();

    return app.exec();
}
