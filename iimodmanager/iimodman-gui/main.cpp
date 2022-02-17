#include "mainwindow.h"

#include <QApplication>
#include <QPushButton>


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  iimodmanager::MainWindow window;
  window.show();

  return app.exec();
}
