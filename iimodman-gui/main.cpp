#include "mainwindow.h"
#include "modmanguiapplication.h"

#include <QApplication>
#include <QPushButton>


int main(int argc, char *argv[])
{
    iimodmanager::ModManGuiApplication app(argc, &argv);

    iimodmanager::MainWindow window(app);
    window.show();

    return app.exec();
}
