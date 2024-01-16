#include "serialportapp.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    SerialPortApp w;
    w.show();
    return a.exec();
}
