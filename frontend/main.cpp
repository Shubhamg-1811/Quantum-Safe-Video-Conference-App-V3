#include "launchwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LaunchWindow window;
    window.show();

    return app.exec();
}
