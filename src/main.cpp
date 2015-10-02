#include "oasisreader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    oasisReader w;
    w.show();

    return app.exec();
}

