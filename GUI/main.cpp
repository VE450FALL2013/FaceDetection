#include <QApplication>

#include "detectdialog.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DetectDialog dialog;
    dialog.show();
    return app.exec();
}
